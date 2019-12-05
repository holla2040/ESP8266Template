#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h> // curl -F "image=@/tmp/arduino_build/ESP8266Template.ino.bin" myLoc.local/upload
#include <WebSocketsServer.h>
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>
#include <PubSubClient.h>


const char logFn[] = "/log.csv";

/*
you need to uncomment these lines in Arduino/libraries/TFT_eSPI/User_Setup.h  
this is config for 1.8" 160x120 color spi display connected to WEMOS mini

#define ST7735_DRIVER
#define TFT_CS   PIN_D8  // Chip select control pin D8
#define TFT_DC   PIN_D3  // Data Command control pin
#define TFT_RST  PIN_D6  // Reset pin (could connect to NodeMCU RST, see next line)


SCL to D5
SDA to D7

*/

#define LOAD_FONT7 

#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

#include "fauxmoESP.h"
#include "project.h"
#include "fs.h"

WiFiUDP   ntpUDP;
NTPClient ntpClient(ntpUDP);
File      logfile;

TFT_eSPI display = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

char line[50];

void setup(void) {
  Serial.begin(115200);
  Serial.println("\nESP8266Template begin");

  fsSetup();
  Serial.println("filesystem started");

  configLoad();

  if (displayEnabled)           displaySetup();

  // wifiManager.resetSettings(); // use this to reset wifimanager creds
  wifiManager.autoConnect(name);

  httpServerSetup();

  if (MDNS.begin(name)) {
      Serial.print ("MDNS responder started http://");
      Serial.print(name);
      Serial.println(".local");
  }
  Serial.println(WiFi.localIP());

  if (tcpServerEnabled)         tcpServerSetup();
  if (websocketserverEnabled)   websocketServerSetup();
  if (heartbeatEnabled)         heartbeatSetup(); 
  if (alexaEnabled)             alexaSetup();
  if (ntpEnabled)               ntpSetup();
  if (loggingEnabled)           loggingSetup();
  if (awsiotEnabled)            awsiotSetup();

  if (displayEnabled) {
    display.setCursor(70, 4, 1);
    display.print(WiFi.localIP());
  }
  Serial.println(TCP_MSS);
}

void loop(void) {
  httpServer.handleClient();

  if (tcpServerEnabled)       tcpServerLoop();
  if (alexaEnabled)           alexaLoop();
  if (ntpEnabled)             ntpLoop();
  if (loggingEnabled)         loggingLoop();
  if (heartbeatEnabled)       heartbeatLoop();
  if (websocketserverEnabled) websocketserverLoop();
  if (displayEnabled)         displayLoop();
  if (awsiotEnabled)          awsiotLoop();

  MDNS.update();
  ESP.wdtFeed(); 
  yield();
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

    heartbeatEnabled        = config["heartbeat"]["enabled"] | 0;
    heartbeatInterval       = config["heartbeat"]["interval"] | 1000;
    heartbeatPin            = config["heartbeat"]["pin"];
    tcpServerEnabled        = config["tcpserver"]["enabled"] | 0;
    websocketserverEnabled  = config["websocketserver"]["enabled"] | 0;
    websocketInterval       = config["websocketserver"]["interval"] | 1000;
    alexaEnabled            = config["alexa"]["enabled"] | 0;
    ntpEnabled              = config["ntp"]["enabled"] | 0;
    ntpOffset               = config["ntp"]["offset"];
    ntpInterval             = config["ntp"]["interval"] | 1000;
    loggingEnabled          = config["logging"]["enabled"] | 0;
    strlcpy(loggingPostURL,config["logging"]["posturl"],POSTURLLEN);
    loggingInterval         = config["logging"]["interval"] | 1000;
    displayEnabled          = config["display"]["enabled"] | 0;
    displayInterval         = config["display"]["interval"] | 1000;
    awsiotEnabled           = config["awsiot"]["enabled"] | 0;

    file.close();
  }
}


/* ---- tcpserver code ----------------------------------------------*/
// use socat TCP:office.local:23 -,raw,echo=0

void tcpServerLoop() {
  int i;
  char c;

  if (tcpServer->hasClient()) {
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      //find free/disconnected spot
      if (!tcpServerClients[i] || !tcpServerClients[i].connected()) {
        if (tcpServerClients[i]) tcpServerClients[i].stop();
        tcpServerClients[i] = tcpServer->available();
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

void tcpServerSetup() {
  if (tcpServerEnabled) {
    int p = config["tcpserver"]["port"];
    tcpServer = new WiFiServer(p);
    tcpServer->setNoDelay(true);
    tcpServer->begin(p);
    Serial.print("tcpServer started on port ");
    Serial.println(p);
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

  httpServer.on("/factory", []() {
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
        publishStatic();
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\r\n", num, payload);
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
    sprintf(line,"uptime:%d",milli/1000);
    webSocketServer->broadcastTXT(line,strlen(line));

    sprintf(line,"led:%d",!digitalRead(heartbeatPin));
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
    sprintf(line,"led:%d",digitalRead(heartbeatPin));
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


/* ---- ntp code ----------------------------------------------*/
void ntpSetup() {
  ntpClient.begin();
  ntpClient.setTimeOffset(ntpOffset);
}

void ntpLoop() {
  uint32_t milli = millis();
  ntpClient.update();
  if (milli > ntpTimeout) {
    sprintf(line,"time:%s",getTimestampString().c_str());
    webSocketServer->broadcastTXT(line,strlen(line));
    ntpTimeout = milli + ntpInterval;
  }
}


/* https://github.com/arduino-libraries/NTPClient/issues/36 */
String getTimestampString() {
   time_t rawtime = ntpClient.getEpochTime();
   struct tm * ti;
   ti = localtime (&rawtime);

   uint16_t year = ti->tm_year + 1900;
   String yearStr = String(year);

   uint8_t month = ti->tm_mon + 1;
   String monthStr = month < 10 ? "0" + String(month) : String(month);

   uint8_t day = ti->tm_mday;
   String dayStr = day < 10 ? "0" + String(day) : String(day);

   uint8_t hours = ti->tm_hour;
   String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

   uint8_t minutes = ti->tm_min;
   String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

   uint8_t seconds = ti->tm_sec;
   String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

   return yearStr + "-" + monthStr + "-" + dayStr + " " +
          hoursStr + ":" + minuteStr + ":" + secondStr;
}

String getTimestampStringShort() {
   time_t rawtime = ntpClient.getEpochTime();
   struct tm * ti;
   ti = localtime (&rawtime);

   uint16_t year = ti->tm_year - 100; // +1900-2000
   String yearStr = String(year);

   uint8_t month = ti->tm_mon + 1;
   String monthStr = month < 10 ? "0" + String(month) : String(month);

   uint8_t day = ti->tm_mday;
   String dayStr = day < 10 ? "0" + String(day) : String(day);

   uint8_t hours = ti->tm_hour;
   String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

   uint8_t minutes = ti->tm_min;
   String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

   uint8_t seconds = ti->tm_sec;
   String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

   return yearStr + monthStr + dayStr + "-" + hoursStr + minuteStr + secondStr;
}

String getDateStringShort() {
   time_t rawtime = ntpClient.getEpochTime();
   struct tm * ti;
   ti = localtime (&rawtime);

   uint16_t year = ti->tm_year - 100; // +1900-2000
   String yearStr = String(year);

   uint8_t month = ti->tm_mon + 1;
   String monthStr = month < 10 ? "0" + String(month) : String(month);

   uint8_t day = ti->tm_mday;
   String dayStr = day < 10 ? "0" + String(day) : String(day);

   return yearStr + monthStr + dayStr;
}

/* ---- logging code ----------------------------------------------*/
void loggingSetup() {
  /* https://tttapa.github.io/ESP8266/Chap16%20-%20Data%20Logging.html */
  /* ~/.arduino15/packages/esp8266/hardware/esp8266/2.5.0/cores/esp8266/FS.h */

  if (SPIFFS.exists(logFn)) {
    logfile = SPIFFS.open(logFn, "a"); // append on exists
  } else {
    logfile = SPIFFS.open(logFn, "w"); // write with line 1 header
    logfile.println("Timestamp,Time,value");
  }
}

void loggingLoop() {
  char status[100];
  int value;

  uint32_t milli = millis();

  if ((ntpClient.getHours() == 23) && (ntpClient.getMinutes() == 59) && (ntpClient.getSeconds() > 50)) {
    loggingPost();
    Serial.println("removing log.csv"); 
    logfile.close();
    delay(10000); // wait until after midnight to start logging again
    logfile = SPIFFS.open(logFn, "w"); // write with line 1 header
    logfile.println("Timestamp,Time,value");
    loggingTimeout = 0;
  }

  if (milli > loggingTimeout) {
    value = random(0,1000);
    sprintf(line,"time:%s",getTimestampString().c_str());
    webSocketServer->broadcastTXT(line,strlen(line));

    sprintf(line,"value:%d",value);
    webSocketServer->broadcastTXT(line,strlen(line));
   
    sprintf(line,"%ld,%s,%d",ntpClient.getEpochTime(),getTimestampString().c_str(),value);
    Serial.print(line);
    Serial.print(" ");
    Serial.println(ESP.getFreeHeap());

    logfile = SPIFFS.open(logFn, "a"); // write with line 1 header
    logfile.println(line);
    logfile.close();

    sprintf(status,"status:%s",line);
    webSocketServer->broadcastTXT(status,strlen(status));

    loggingTimeout = milli + loggingInterval;
  }
}

void loggingPost() {
  if (strlen(loggingPostURL)) {
    HTTPClient http;
    char uploadName[50];
    sprintf(uploadName,"%s-%s-log.csv",name,getTimestampStringShort().c_str());

    logfile = SPIFFS.open(logFn, "r"); 
    size_t contentLength = logfile.size();

    http.begin(loggingPostURL);
    http.addHeader("Content-Type", "application/octet-stream");
    http.addHeader("Content-Length", String(contentLength));
    http.addHeader("Filename", uploadName);
    int httpCode = http.sendRequest("POST",&logfile,contentLength);

/*
    Serial.println("loggingPost");
    Serial.println(loggingPostURL);
    Serial.println(contentLength);

    if (httpCode > 0) {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    Serial.print("httpCode ");
    Serial.println(httpCode);
*/
    
    http.end();
    logfile.close();
    String fn = "/log-"+getDateStringShort()+".csv";
    Serial.println(SPIFFS.rename("/log.csv",fn.c_str()));
/*
    if (httpCode != 200) {
      SPIFFS.rename("/log.csv",uploadName);
    }
*/
  }
}



/* ---- status code ----------------------------------------------*/

void publishStatic() {
    sprintf(line,"name:%s",name);
    webSocketServer->broadcastTXT(line,strlen(line));
    sprintf(line,"label:%s",label);
    webSocketServer->broadcastTXT(line,strlen(line));
}


/* ---- display code ----------------------------------------------*/
void displaySetup() {
  display.init();
  display.setRotation(1);
  display.fillScreen(TFT_BLACK);
  display.setTextColor(TFT_WHITE,TFT_BLACK);  
  display.setCursor(2, 4, 1);
  display.print(name);
}

void displayLoop() {
  uint32_t milli = millis();
  if (milli > displayTimeout) {
    display.setCursor(2, 14, 2);
    display.print(getTimestampString()); 
    displayTimeout = milli + displayInterval;
    display.setCursor(0, 34, 8);
    display.print(millis()/1000);
    display.setTextSize(1);
  }
}

/* ---- awsiot code ----------------------------------------------*/
void awsiotHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

WiFiClientSecure espClient;
PubSubClient     awsiotClient(espClient); 

void awsiotSetup() {
  // awsiotClient.setCallback(awsiotEndpoint, 8883, awsiotHandler, 
}

void awsiotLoop() {
}
