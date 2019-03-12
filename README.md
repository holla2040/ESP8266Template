# ESP8266Template
Here's my ESP8266 template. 
Supports
* ESP8266WebServer
* ESP8266HTTPUpdateServer
* WiFiManager
* MDNS
* WebSockets
* Non-delay led blink with heartbeat interval config.json support
* SPIFFS with FSBrowserNG
* ArduinoJson
* config.json parsing, see data/config.json
* index.htm has ajax, charting, websocket example code
* configurable TCP Socket - Serial Port bridging
* configurable websocket server
* configurable LED heartbeat
* configurable alexa compatibility using fauxmoESP

Sample URLs (also look at the curl make targets)
* status query,http://esp8266.local/status
* reset wifi manager, http://esp8266.local/reset
* reboot, http://esp8266.local/reboot
* changing runtime hearbeat interval, http://esp8266.local/heartbeat?interval=50

Requires the following libraries to be installed, tested with arduino IDE 1.8.8
* https://github.com/Links2004/arduinoWebSockets
* https://github.com/tzapu/WiFiManager
* https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
* https://github.com/me-no-dev/ESPAsyncTCP
# https://bitbucket.org/xoseperez/fauxmoesp
