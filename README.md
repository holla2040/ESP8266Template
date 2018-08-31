# ESP8266Template
Here's my ESP8266 template. 
Supports
* ESP8266WebServer
* ESP8266HTTPUpdateServer
* WiFiManager
* MDNS
* WebSockets
* Non-delay led blink with heartbeat interval config.json support
* SPIFFS
* ArduinoJson
* config.json parsing
* index.htm has ajax and websocket example code
* option compile of TCP Socket - Serial Port bridging

Sample URLs (also look at the curl make targets)
* status query,http://esp8266.local/status
* reset wifi manager, http://esp8266.local/reset
* reboot, http://esp8266.local/reboot
* changing runtime hearbeat interval, http://esp8266.local/heartbeat?value=50
