# copy 2 arduino IDE arduino-builder lines to bin
# delete the unique numbers from build and cache directory, these are unique to IDE runs
# :s/_69751//g
# :s/_90469//g
# if you want quieter output, 
#  delete -dump-prefs -verbose
#  add -quiet=1
# hopefully arduino-cli will eliminate this hack
# or use, https://github.com/plerup/makeEspArduino
#
# passing 'name' argument to make
# make reboot name=esp8266

ino = $(wildcard *.ino)
name = 192.168.0.114
port = /dev/ttyUSB0
arduinodir = /home/holla/arduino
arduinosketch = /home/holla/Arduino
arduinosdir15 = /home/holla/.arduino15

try:
	@mkdir -p /tmp/arduino_build /tmp/arduino_cache
	${arduinodir}/arduino-builder -dump-prefs -logger=machine -hardware ${arduinodir}/hardware -hardware ${arduinosdir15}/packages -hardware ${arduinosketch}/hardware -tools ${arduinodir}/tools-builder -tools ${arduinodir}/hardware/tools/avr -tools ${arduinosdir15}/packages -built-in-libraries ${arduinodir}/libraries -libraries ${arduinosketch}/libraries -fqbn=esp8266:esp8266:d1_mini:CpuFrequency=80,VTable=flash,FlashSize=4M1M,LwIPVariant=v2mss1460,Debug=Disabled,DebugLevel=None____,FlashErase=none,UploadSpeed=921600 -ide-version=10806 -build-path /tmp/arduino_build -warnings=none -build-cache /tmp/arduino_cache -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.mkspiffs.path=${arduinosdir15}/packages/esp8266/tools/mkspiffs/0.2.0 -prefs=runtime.tools.esptool.path=${arduinosdir15}/packages/esp8266/tools/esptool/0.4.13 -prefs=runtime.tools.xtensa-lx106-elf-gcc.path=${arduinosdir15}/packages/esp8266/tools/xtensa-lx106-elf-gcc/1.20.0-26-gb404fb9-2 -verbose ./$(ino)
	${arduinodir}/arduino-builder -compile -logger=machine -hardware ${arduinodir}/hardware -hardware ${arduinosdir15}/packages -hardware ${arduinosketch}/hardware -tools ${arduinodir}/tools-builder -tools ${arduinodir}/hardware/tools/avr -tools ${arduinosdir15}/packages -built-in-libraries ${arduinodir}/libraries -libraries ${arduinosketch}/libraries -fqbn=esp8266:esp8266:d1_mini:CpuFrequency=80,VTable=flash,FlashSize=4M1M,LwIPVariant=v2mss1460,Debug=Disabled,DebugLevel=None____,FlashErase=none,UploadSpeed=921600 -ide-version=10806 -build-path /tmp/arduino_build -warnings=none -build-cache /tmp/arduino_cache -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.mkspiffs.path=${arduinosdir15}/packages/esp8266/tools/mkspiffs/0.2.0 -prefs=runtime.tools.esptool.path=${arduinosdir15}/packages/esp8266/tools/esptool/0.4.13 -prefs=runtime.tools.xtensa-lx106-elf-gcc.path=${arduinosdir15}/packages/esp8266/tools/xtensa-lx106-elf-gcc/1.20.0-26-gb404fb9-2 -verbose ./$(ino)

run: bin
	/home/holla/.arduino15/packages/esp8266/tools/esptool/0.4.13/esptool -vv -cd nodemcu -cb 921600 -cp $(port) -ca 0x00000 -cf /tmp/arduino_build/$(ino).bin 

flash: 
	@- pkill -9 -f microcom
	/home/holla/.arduino15/packages/esp8266/tools/esptool/0.4.13/esptool -vv -cd nodemcu -cb 921600 -cp $(port) -ca 0x00000 -cf /tmp/arduino_build/$(ino).bin 
	@- pkill -9 -f sleep

files:
	@name=${name} ./transferdata.sh
	@echo ${name}
	curl http://${name}/reload

reload:
	@echo ${name}
	@echo "reload"
	curl http://${name}/reload

ota:
	@echo ${name}
	@echo "ota"
	curl -F "image=@/tmp/arduino_build/$(ino).bin" http://${name}/upload

reboot:
	@echo ${name}
	@echo "reboot"
	curl http://${name}/reboot

reset:
	@echo ${name}
	@echo "uploading reseting"
	curl http://${name}/reset

index:
	@echo ${name}
	@echo "uploading index.htm"
	@curl -F "file=@./data/index.htm" http://${name}/edit

config:
	@echo ${name}
	@echo "uploading config.json"
	@curl -F "file=@./data/config.json" http://${name}/edit
	@curl http://${name}/config.json

clean:
	rm -rf /tmp/arduino_build*
	rm -rf /tmp/arduino_cache*
