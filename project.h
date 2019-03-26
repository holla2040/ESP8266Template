#ifndef PROJECT_H
#define PROJECT_H

#define NAMELEN 20
#define LABELLEN 50

char name[NAMELEN];
char label[LABELLEN];

StaticJsonDocument<1024> doc;
JsonObject config;


ESP8266WebServer httpServer(80);
void httpServerSetup();
ESP8266HTTPUpdateServer httpUpdater;
const char *update_path = "/upload";
WiFiManager wifiManager;
void configLoad();

boolean     tcpServerEnabled; 
#define     MAX_SRV_CLIENTS 4
WiFiServer  *tcpServer;
WiFiClient  tcpServerClients[MAX_SRV_CLIENTS];
void        tcpServerSetup();
void        tcpServerLoop();

boolean     heartbeatEnabled;
uint32_t    heartbeatTimeout;
uint8_t     heartbeatPin;
uint32_t    heartbeatInterval; // this is heartbeat interval in mS
void        heartbeatSetup();

boolean     websocketserverEnabled;
uint32_t    websocketTimeout;
uint32_t    websocketInterval; 
WebSocketsServer *webSocketServer;
void        websocketServerSetup();
void        websocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);

boolean     alexaEnabled;
fauxmoESP   *alexa;
void        alexaSetup();
void        alexaLoop();

uint32_t    ncTimeout;
void        ncLoop();

boolean     ntpEnabled;
long        ntpOffset;
uint32_t    ntpTimeout;
uint16_t    ntpInterval; 
String      getTimestampString();

boolean     loggingEnabled;
uint32_t    loggingTimeout;
uint16_t    loggingInterval;

boolean     displayEnabled;
uint32_t    displayTimeout;
uint16_t    displayInterval;


#endif
