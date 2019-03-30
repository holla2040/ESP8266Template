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
* configurable NTP compatibility using NTPClient
* SPIFFS csv log file support with daily reset
* TFT_eSPI for various graphical displays
* supports pre-midnight log file HTTP POST to external site
* support for AWS IoT MQTT using x.509 certs and TLSv1.2

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
* https://bitbucket.org/xoseperez/fauxmoesp
* https://github.com/arduino-libraries/NTPClient
* https://github.com/Bodmer/TFT_eSPI
* https://github.com/copercini/esp8266-aws_iot

AWS IoT Notes
* Login to AWS Management console, navigate to IoT Core
* Create a single thing, give it name, type sensor 
* One-click certification creation
* Download certificate, private, public keys and Amazon root CA
* Create 3 'der' files
  * openssl x509 -in  ######-certificate.pem.crt -out cert.der -outform DER
  * openssl rsa -in ######-private.pem.key -out private.der -outform DER 
  * openssl x509 -in AmazonRootCA1.pem -out ca.der -outform DER 
* Upload cert.der, private.der and ca.der to device's SPIFFS partition
* Open 'Test' interface from left navigation list
* View and copy endpoint from upper-right 'View endpoint' pop-up menu
* Add endpoint to config.json using HTML editor

