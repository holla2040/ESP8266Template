#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h> // curl -F "image=@/tmp/arduino_build/ESP8266Template.ino.bin" myLoc.local/upload
#include <WebSocketsServer.h>
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager
#include <EEPROM.h>
#include <ArduinoJson.h>

/* todo
  need to ditch these functionality defines and 'enable' them in the config file
*/


#define ALEXA
#define TCPSERVER
#define WEBSOCKETSERVER

/*
  upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload the contents of a folder if you CD in that folder and run the following command:
  for file in `ls -A1`; do echo $file;curl -F "file=@$PWD/$file" myLoc.local/edit; done
*/

// comment to disable TCP Socket Server,  socat TCP:office.local:23 -,raw,echo=0
#define MAX_SRV_CLIENTS 4
#define TCPSERVERPORT 23

#define NAMELEN 20
#define LABELLEN 50
#define LED 2

char name[NAMELEN];
char label[LABELLEN];
uint32_t heartbeatTimeout;
uint32_t heartbeat; // this is heartbeat interval in mS

ESP8266WebServer httpServer(80);
StaticJsonDocument<1024> doc;
JsonObject config;

#ifdef WEBSOCKETSERVER
WebSocketsServer webSocketServer = WebSocketsServer(81);
#endif

#ifdef TCPSERVER
WiFiServer tcpServer(TCPSERVERPORT);
WiFiClient tcpServerClients[MAX_SRV_CLIENTS];
#endif

#ifdef ALEXA
#include "fauxmoESP.h"
fauxmoESP alexa;
#endif


ESP8266HTTPUpdateServer httpUpdater;
const char *update_path = "/upload";
WiFiManager wifiManager;
#include "fs.h"

#ifdef WEBSOCKETSERVER
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocketServer.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\r\n", num, payload);

/*
      if (strcmp(LEDON, (const char *)payload) == 0) {
        writeLED(true);
      }
      else if (strcmp(LEDOFF, (const char *)payload) == 0) {
        writeLED(false);
      }
      else {
        Serial.println("Unknown command");
      }
*/

      // send data to all connected clients
      webSocketServer.broadcastTXT(payload, length);
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      hexdump(payload, length);

      // echo data back to browser
      webSocketServer.sendBIN(num, payload, length);
      break;
    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}
#endif


//get heap status, analog input value and all GPIO statuses in one json call
void handleStatus() {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    httpServer.send(200, "text/json", json);
    json = String();
}

void configLoad() {
  File file = SPIFFS.open("/config.json", "r");
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println(F("Failed to read file, using default configuration"));
  } else {
    config = doc.as<JsonObject>();
    strlcpy(name,config["name"] | "myName", NAMELEN);
    Serial.print("name:      ");
    Serial.println(name);

    strlcpy(label,config["label"] | "myLabel", LABELLEN);
    Serial.print("label:     ");
    Serial.println(label);

    heartbeat = config["heartbeat"] | 1001;
    Serial.print("heartbeat: ");
    Serial.println(heartbeat);

    file.close();
  }
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += httpServer.uri();
  message += "\nMethod: ";
  message += (httpServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += httpServer.args();
  message += "\n";

  for (uint8_t i = 0; i < httpServer.args(); i++) {
    message += " " + httpServer.argName(i) + ": " + httpServer.arg(i) + "\n";
  }

  httpServer.send(404, "text/plain", message);
}

void setup(void) {
  char c;
  int i;
  Serial.begin(115200);
  Serial.println("\nESP8266Template begin");
  pinMode(LED, OUTPUT);

  fsSetup();
  Serial.println("filesystem started");

  /* default values, updated by config.json parsing */
  heartbeat = 1001;
  strcpy(name,"myLoc");

  configLoad();

  // wifiManager.resetSettings();
  wifiManager.autoConnect(name);

  httpServer.on("/status", HTTP_GET, handleStatus );
  httpServer.on("/reset", []() {
    httpServer.send(200, "text/plain", "reseting wifi settings\n");
    wifiManager.resetSettings();
  });
  httpServer.on("/reboot", []() {
    httpServer.send(200, "text/plain", "rebooting\n");
    delay(1000);
    ESP.restart();
  });
  httpServer.on("/reload", []() {
    httpServer.send(200, "text/plain", "reloading\n");
    configLoad();
  });
  httpServer.on("/heartbeat", []() {
    httpServer.send(200, "text/plain", "heartbeat set\n");
    heartbeat = (httpServer.arg("value")).toInt();
  });


  httpUpdater.setup(&httpServer, update_path);
  httpServer.begin();
  // Serial.print("IP   ");
  // Serial.println(WiFi.localIP().toString());
  Serial.println("HTTP server started");
  Serial.println("HTTP updater started");

  if (MDNS.begin(name)) {
      Serial.print ("MDNS responder started http://");
      Serial.print(name);
      Serial.println(".local");
  }

#ifdef WEBSOCKETSERVER
  webSocketServer.begin();
  webSocketServer.onEvent(webSocketEvent);
  Serial.println("WebSocketServer started");
#endif

#ifdef TCPSERVER
  tcpServer.setNoDelay(true);
  tcpServer.begin();
  Serial.print("tcpServer started on port ");
  Serial.println(TCPSERVERPORT);
#endif

#ifdef ALEXA
  alexa.enable(true);
  alexa.enable(false);
  alexa.enable(true);

  Serial.print("alexa devices '");
  // from https://arduinojson.org/v6/api/jsonarray/
  for(JsonVariant v : config["alexa"].as<JsonArray>()) {
    Serial.print(v.as<String>());
    Serial.print("' ");
    (v.as<String>()).toCharArray(name,19);
    alexa.addDevice(name);
  }
  Serial.println("started");

  alexa.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
    Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
    digitalWrite(LED, !state);
  });
#endif

}

#ifdef TCPSERVER
void tcpServerLoop() {
  int i;
  char c;
  if (tcpServer.hasClient()) {
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      //find free/disconnected spot
      if (!tcpServerClients[i] || !tcpServerClients[i].connected()) {
        if (tcpServerClients[i]) tcpServerClients[i].stop();
        tcpServerClients[i] = tcpServer.available();
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient tcpServerClient = tcpServer.available();
    tcpServerClient.stop();
  }

  for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (tcpServerClients[i] && tcpServerClients[i].connected()) {
      while (tcpServerClients[i].available()) {
        Serial.print((char)tcpServerClients[i].read());
      }
    }
  }

  if (Serial.available()) {
    c = Serial.read();
    for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (tcpServerClients[i] && tcpServerClients[i].connected()) {
        tcpServerClients[i].write(c);
      }
    }
  }
}

void tcpServerWrite(char *buf, uint16_t len) {
  for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (tcpServerClients[i] && tcpServerClients[i].connected()) {
      tcpServerClients[i].write(buf,len);
    }
  }
}

#endif
  

void loop(void) {
  char line[40];
  httpServer.handleClient();

#ifdef WEBSOCKETSERVER
  webSocketServer.loop();
#endif 

#ifdef TCPSERVER
  tcpServerLoop();
#endif

#ifdef ALEXA
  alexa.handle();
#endif

  if (millis() > heartbeatTimeout) {
    digitalWrite(LED,!digitalRead(LED));

    sprintf(line,"uptime:%d",millis()/25);
#ifdef WEBSOCKETSERVER
    webSocketServer.broadcastTXT(line,strlen(line));
#endif

    sprintf(line,"led:%d",!digitalRead(LED));
#ifdef WEBSOCKETSERVER
    webSocketServer.broadcastTXT(line,strlen(line));
#endif

    sprintf(line,"name:%s",name);
#ifdef WEBSOCKETSERVER
    webSocketServer.broadcastTXT(line,strlen(line));
#endif

    sprintf(line,"label:%s",label);
#ifdef WEBSOCKETSERVER
    webSocketServer.broadcastTXT(line,strlen(line));
#endif

    sprintf(line,"uptime:%d",millis()/25);
#ifdef WEBSOCKETSERVER
    webSocketServer.broadcastTXT(line,strlen(line));
#endif

    heartbeatTimeout = millis() + heartbeat;
  }

  ESP.wdtFeed(); 

  yield();
}
