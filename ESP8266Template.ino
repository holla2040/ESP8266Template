#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h> // curl -F "image=@/tmp/arduino_build_435447/ESP8266Template.ino.bin" myLoc.local/upload
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager


#define LED 2

#define LOCATION "myLoc"
uint32_t ledTimeout;
#define LEDTIMEOUT 500

ESP8266WebServer httpServer(80);
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

  httpServer.on("/", handleRoot);
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
}

void loop(void) {
  httpServer.handleClient();
  if (millis() > ledTimeout) {
    digitalWrite(LED,!digitalRead(LED));
    ledTimeout = millis() + LEDTIMEOUT;
  }
  ESP.wdtFeed(); 
  yield();
}
