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
name = myLoc
port = /dev/ttyUSB0

bin:
	@mkdir -p /tmp/arduino_build /tmp/arduino_cache
	/home/holla/arduino-1.8.6/arduino-builder -dump-prefs -logger=machine -hardware /home/holla/arduino-1.8.6/hardware -hardware /home/holla/.arduino15/packages -hardware /home/holla/Arduino/hardware -tools /home/holla/arduino-1.8.6/tools-builder -tools /home/holla/arduino-1.8.6/hardware/tools/avr -tools /home/holla/.arduino15/packages -built-in-libraries /home/holla/arduino-1.8.6/libraries -libraries /home/holla/Arduino/libraries -fqbn=esp8266:esp8266:d1_mini:CpuFrequency=80,VTable=flash,FlashSize=4M1M,LwIPVariant=v2mss536,Debug=Disabled,DebugLevel=None____,FlashErase=none,UploadSpeed=921600 -ide-version=10806 -build-path /tmp/arduino_build -warnings=none -build-cache /tmp/arduino_cache -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.mkspiffs.path=/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/0.2.0 -prefs=runtime.tools.esptool.path=/home/holla/.arduino15/packages/esp8266/tools/esptool/0.4.13 -prefs=runtime.tools.xtensa-lx106-elf-gcc.path=/home/holla/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/1.20.0-26-gb404fb9-2 -verbose ./$(ino)
	/home/holla/arduino-1.8.6/arduino-builder -compile -logger=machine -hardware /home/holla/arduino-1.8.6/hardware -hardware /home/holla/.arduino15/packages -hardware /home/holla/Arduino/hardware -tools /home/holla/arduino-1.8.6/tools-builder -tools /home/holla/arduino-1.8.6/hardware/tools/avr -tools /home/holla/.arduino15/packages -built-in-libraries /home/holla/arduino-1.8.6/libraries -libraries /home/holla/Arduino/libraries -fqbn=esp8266:esp8266:d1_mini:CpuFrequency=80,VTable=flash,FlashSize=4M1M,LwIPVariant=v2mss536,Debug=Disabled,DebugLevel=None____,FlashErase=none,UploadSpeed=921600 -ide-version=10806 -build-path /tmp/arduino_build -warnings=none -build-cache /tmp/arduino_cache -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.mkspiffs.path=/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/0.2.0 -prefs=runtime.tools.esptool.path=/home/holla/.arduino15/packages/esp8266/tools/esptool/0.4.13 -prefs=runtime.tools.xtensa-lx106-elf-gcc.path=/home/holla/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/1.20.0-26-gb404fb9-2 -verbose ./$(ino)



run: bin
	/home/holla/.arduino15/packages/esp8266/tools/esptool/0.4.13/esptool -vv -cd nodemcu -cb 921600 -cp $(port) -ca 0x00000 -cf /tmp/arduino_build/$(ino).bin 

flash: 
	/home/holla/.arduino15/packages/esp8266/tools/esptool/0.4.13/esptool -vv -cd nodemcu -cb 921600 -cp $(port) -ca 0x00000 -cf /tmp/arduino_build/$(ino).bin 

files:
	@./transferdata.sh
	@echo ${name}.local
	curl http://${name}.local/reload

reload:
	@echo ${name}.local
	@echo "reload"
	curl http://${name}.local/reload

ota:
	@echo ${name}.local
	@echo "ota"
	curl -F "image=@/tmp/arduino_build/$(ino).bin" http://${name}.local/upload

reboot:
	@echo ${name}.local
	@echo "reboot"
	curl http://${name}.local/reboot

reset:
	@echo ${name}.local
	@echo "uploading reseting"
	curl http://${name}.local/reset

index:
	@echo ${name}.local
	@echo "uploading index.htm"
	@curl -F "file=@./data/index.htm" http://${name}.local/edit

config:
	@echo ${name}.local
	@echo "uploading config.json"
	@curl -F "file=@./data/config.json" http://${name}.local/edit
	@curl http://${name}.local/config.json

clean:
	rm -rf /tmp/arduino_build*
	rm -rf /tmp/arduino_cache*
