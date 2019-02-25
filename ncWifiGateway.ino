#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h> // curl -F "image=@/tmp/arduino_build/ESP8266Template.ino.bin" myLoc.local/upload
#include <WebSocketsServer.h>
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager
#include <EEPROM.h>
#include <ArduinoJson.h>

#include "fauxmoESP.h"
#include "project.h"
#include "fs.h"

// nc -C 192.168.1.117 23
// $N00@]
// nulsom needs null modem

// #define DEBUG 


char tcpbuffer[20];
char ncReply[20];

void setup(void) {
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("\nncWifiGateway setup");
#endif

  fsSetup();
#ifdef DEBUG
  Serial.println("filesystem started");
#endif

  configLoad();

  // wifiManager.resetSettings();
  wifiManager.autoConnect(name);

  httpServerSetup();

  if (MDNS.begin(name)) {
#ifdef DEBUG
      Serial.print ("MDNS responder started http://");
      Serial.print(name);
      Serial.println(".local");
#endif
  }

  if (tcpServerEnabled)         tcpServerSetup();
  if (websocketserverEnabled)   websocketServerSetup();
  if (heartbeatEnabled)         heartbeatSetup(); 
  if (alexaEnabled)             alexaSetup();

#ifdef DEBUG
  Serial.println("\nncWifiGateway setup complete, switching to 9600 7E1");
  delay(500);
#endif
  Serial.begin(9600, SERIAL_7E1);
  Serial.print('\r');
  Serial.print('\r');
  Serial.print('\r');
}


void loop(void) {
  httpServer.handleClient();

  if (tcpServerEnabled)       tcpServerLoop();
  if (alexaEnabled)           alexaLoop();
  if (heartbeatEnabled)       heartbeatLoop();
  if (websocketserverEnabled) websocketserverLoop();
  ncLoop();

  ESP.wdtFeed(); 
  yield();
}

uint8_t commandChecksum(char *command) {
  uint8_t d7d6, d1d0, _xor, chk = 0;
  while (*command) {
    chk += command[0];
    command++;
  }
  d7d6 = chk >> 6;
  d1d0 = chk & 0x03;
  _xor = d7d6 ^ d1d0;
  chk = (((chk & 0xFC) + _xor) & 0x3F) + 0x30;
  return chk;
}



/* ---- config code ----------------------------------------------*/
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

    heartbeatEnabled        = config["heartbeat"]["enabled"];
    heartbeatInterval       = config["heartbeat"]["interval"] | 1000;
    heartbeatPin            = config["heartbeat"]["pin"];
    tcpServerEnabled        = config["tcpserver"]["enabled"];
    websocketserverEnabled  = config["websocketserver"]["enabled"];
    websocketInterval       = config["websocketserver"]["interval"] | 1000;
    alexaEnabled            = config["alexa"]["enabled"];

    file.close();
    Serial.println("configLoad done");
    Serial.println(heartbeatPin);
  }
}



/* ---- tcpserver code ----------------------------------------------*/
// use socat TCP:office.local:23 -,raw,echo=0


void tcpServerLoop() {
  int i;
  char c;
  uint8_t chk;

  if (tcpServer->hasClient()) {
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      //find free/disconnected spot
      if (!tcpServerClients[i] || !tcpServerClients[i].connected()) {
        if (tcpServerClients[i]) tcpServerClients[i].stop();
        tcpServerClients[i] = tcpServer->available();
        tcpServerClients[i].print("ncWifiGateway connected\n");
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient tcpServerClient = tcpServer->available();
    tcpServerClient.stop();
  }

  for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (tcpServerClients[i] && tcpServerClients[i].connected()) {
      while (tcpServerClients[i].available()) {
        c = tcpServerClients[i].read();
        if (c == '\n') {
          Serial.print('$');
          Serial.print(tcpbuffer);
          chk = commandChecksum(tcpbuffer);
          Serial.print((char)chk);
          Serial.print('\r');
          strcpy(tcpbuffer,"");
        } else {
          int l = strlen(tcpbuffer);
          tcpbuffer[l] = c;
          tcpbuffer[l+1] = 0;
          if (strlen(tcpbuffer) > 18) strcpy(tcpbuffer,"$");
        }
      }
    }
  }

  if (Serial.available()) {
    c = Serial.read();
    for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (tcpServerClients[i] && tcpServerClients[i].connected()) {
        if (c == '\r') {
          tcpServerClients[i].write('\n');
        } else {
//          tcpServerClients[i].write(' ');
          tcpServerClients[i].write(c);
//          tcpServerClients[i].print("[");
//          tcpServerClients[i].print(c,HEX);
//          tcpServerClients[i].print("]");
        }
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

void tcpServerSetup() {
  if (tcpServerEnabled) {
    int p = config["tcpserver"]["port"];
    tcpServer = new WiFiServer(p);
    tcpServer->setNoDelay(true);
    tcpServer->begin(p);
    Serial.print("tcpServer started on port ");
    Serial.println(p);
    strcpy(tcpbuffer,"");
  }
}



/* ---- httpServer code ----------------------------------------------*/
void httpServerSetup() {
  httpServer.on("/heartbeat", []() {
    httpServer.send(200, "text/plain", "heartbeat interval set\n");
    if (httpServer.hasArg("interval")) {
      heartbeatInterval = (httpServer.arg("interval")).toInt();
      if (heartbeatInterval < 1) {
        heartbeatInterval = 10000;
      }
    }
    if (httpServer.hasArg("enabled")) {
      heartbeatEnabled = (httpServer.arg("enabled")).toInt();
    }
    heartbeatTimeout = 0;
  });

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

  httpUpdater.setup(&httpServer, update_path);
  httpServer.begin();
  // Serial.print("IP   ");
  // Serial.println(WiFi.localIP().toString());
  Serial.println("HTTP server started");
  Serial.println("HTTP updater started");
}

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




/* ---- websocket code ----------------------------------------------*/
void websocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocketServer->remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        websocketTimeout = millis() + 10;
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
      webSocketServer->broadcastTXT(payload, length);
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      hexdump(payload, length);

      // echo data back to browser
      webSocketServer->sendBIN(num, payload, length);
      break;
    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}

void websocketServerSetup() {
    webSocketServer = new WebSocketsServer(81);
    webSocketServer->begin();
    webSocketServer->onEvent(websocketEvent);
    Serial.println("WebSocketServer started");
}

void websocketserverLoop() {
  uint32_t milli = millis();
  webSocketServer->loop();
  if (milli > websocketTimeout) {
    char line[40];
    sprintf(line,"uptime:%d",milli/1000);
    webSocketServer->broadcastTXT(line,strlen(line));
    websocketTimeout = milli + websocketInterval;
  }
}

/* ---- heartbeat code ----------------------------------------------*/
void heartbeatSetup() {
    pinMode(heartbeatPin, OUTPUT);
}

void heartbeatLoop() {
  uint32_t milli = millis();
  if (milli > heartbeatTimeout) {
    digitalWrite(heartbeatPin,!digitalRead(heartbeatPin));
    heartbeatTimeout = milli + heartbeatInterval;
  }
}

/* ---- alexa code ----------------------------------------------*/
void alexaSetup() {
    char devname[NAMELEN];
    alexa = new fauxmoESP();
    alexa->enable(true);
    alexa->enable(false);
    alexa->enable(true);

    Serial.print("alexa devices ");
    // from https://arduinojson.org/v6/api/jsonarray/
    for(JsonVariant v : config["alexa"]["devices"].as<JsonArray>()) {
      Serial.print("'");
      Serial.print(v.as<String>());
      Serial.print("' ");
      (v.as<String>()).toCharArray(devname,NAMELEN-1);
      alexa->addDevice(devname);
    }
    Serial.println("started");

    alexa->onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
      Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
      digitalWrite(heartbeatPin, !state);
    });
}

void alexaLoop() {
  alexa->handle();  
}


/*
  upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload the contents of a folder if you CD in that folder and run the following command:
  for file in `ls -A1`; do echo $file;curl -F "file=@$PWD/$file" myLoc.local/edit; done
*/


// nc specific functions

int ncCommandSend(char *command) {
  uint32_t timeout;
  char query[20];
  char c;
  char *ptr = &ncReply[0];
  uint8_t chk = commandChecksum(command);
  sprintf(query, "$%s%c\r", command, (char)chk);
  Serial.print(query);
  timeout = millis() + 200;
  while (millis() < timeout) {
    if (Serial.available()) {
      c = Serial.read();
      if (c == '\r') {
        *ptr = 0;
        return strlen(ncReply);
      } else {
        *ptr++ = c;
      }
    }
  }
  return 0; 
}

void ncCommandPublish(char *command,char *wsid) {
  char line[40];
  if (ncCommandSend(command)) {
    ncReply[strlen(ncReply)-1] = 0; // clear off the checksum byte 
    sprintf(line,"%s:%s",wsid,&ncReply[2]);
    webSocketServer->broadcastTXT(line,strlen(line));
/*
    for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (tcpServerClients[i] && tcpServerClients[i].connected()) {
        tcpServerClients[i].println(line);
      }
    }
*/
  }
}


void ncLoop() {
  uint32_t milli = millis();
  if (milli > ncTimeout) {
    ncCommandPublish("N@","nctyperev");
    ncCommandPublish("P19J","p19stage1t");
    ncCommandPublish("P19K","p19stage2t");
    ncCommandPublish("P19L","p19vacuum");
    ncCommandPublish("P19A?","p19on");
    ncCommandPublish("P19B?","p19tcon");
    ncCommandPublish("P19D?","p19rough");
    ncCommandPublish("P19E?","p19purge");
    ncTimeout = milli + 1000;
  }
}
