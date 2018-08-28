#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h> // curl -F "image=@/tmp/arduino_build_435447/ESP8266Template.ino.bin" myLoc.local/upload
#include <WebSocketsServer.h>
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager

/*
  upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload the contents of a folder if you CD in that folder and run the following command:
  for file in `ls -A1`; do echo $file;curl -F "file=@$PWD/$file" myLoc.local/edit; done
*/

#define LED 2

#define LOCATION "myLoc"
uint32_t ledTimeout;
#define LEDTIMEOUT 500

ESP8266WebServer httpServer(80);
WebSocketsServer webSocketServer = WebSocketsServer(81);

ESP8266HTTPUpdateServer httpUpdater;
const char *update_path = "/upload";
WiFiManager wifiManager;
#include "fs.h"

void handleRoot() {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400, "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>%s Data</title>\
    <style>\
      body { }\
    </style>\
  </head>\
  <body>\
    <pre>\
    Location:    %s<br>\
    Uptime:      %02d:%02d:%02d<br>\
    </pre>\
  </body>\
</html>", LOCATION, LOCATION, hr, min % 60, sec % 60);
  httpServer.send(200, "text/html", temp);
}

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
/*
        // Send the current LED status
        if (LEDStatus) {
          webSocketServer.sendTXT(num, LEDON, strlen(LEDON));
        }
        else {
          webSocketServer.sendTXT(num, LEDOFF, strlen(LEDOFF));
        }
*/
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
      // send data to all connected clients
*/
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

void setup(void) {
  Serial.begin(115200);
  Serial.println("\nESP8266Template begin");
  pinMode(LED, OUTPUT);

  // wifiManager.resetSettings();
  wifiManager.autoConnect(LOCATION);

  httpServer.on("/status", HTTP_GET, handleStatus );
  httpServer.on("/reset", []() {
    httpServer.send(200, "text/plain", "reseting config and hardware\n");
    wifiManager.resetSettings();
    ESP.reset();
  });

  fsSetup();

  httpUpdater.setup(&httpServer, update_path);
  httpServer.begin();
  // Serial.print("IP   ");
  // Serial.println(WiFi.localIP().toString());
  Serial.println("HTTP server started");
  Serial.println("HTTP updater started");
  if (MDNS.begin(LOCATION)) {
      Serial.print ("MDNS responder started http://");
      Serial.print(LOCATION);
      Serial.println(".local");
  }
  webSocketServer.begin();
  webSocketServer.onEvent(webSocketEvent);
}

void loop(void) {
  httpServer.handleClient();
  webSocketServer.loop();

  if (millis() > ledTimeout) {
    char line[20];
    digitalWrite(LED,!digitalRead(LED));

    sprintf(line,"uptime:%d",millis()/25);
    webSocketServer.broadcastTXT(line,strlen(line));

    sprintf(line,"led:%d",!digitalRead(LED));
    webSocketServer.broadcastTXT(line,strlen(line));

    ledTimeout = millis() + LEDTIMEOUT;
  }

  ESP.wdtFeed(); 
  yield();
}
