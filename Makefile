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

ifeq ($(port),)
port := /dev/ttyUSB0
endif

ifeq ($(name),)
name := 192.168.1.117
endif

arduinodir = /home/holla/arduino
arduinosketch = /home/holla/Arduino
arduinosdir15 = /home/holla/.arduino15

try:
	@mkdir -p /tmp/arduino_build /tmp/arduino_cache
	/home/holla/arduino/arduino-builder -dump-prefs -logger=machine -hardware /home/holla/arduino/hardware -hardware /home/holla/.arduino15/packages -hardware /home/holla/Arduino/hardware -tools /home/holla/arduino/tools-builder -tools /home/holla/arduino/hardware/tools/avr -tools /home/holla/.arduino15/packages -built-in-libraries /home/holla/arduino/libraries -libraries /home/holla/Arduino/libraries -fqbn=esp8266:esp8266:d1_mini:xtal=80,vt=flash,exception=disabled,eesz=4M3M,ip=lm2f,dbg=Disabled,lvl=None____,wipe=none,baud=921600 -ide-version=10808 -build-path /tmp/arduino_build -warnings=none -build-cache /tmp/arduino_cache -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.mkspiffs.path=/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/2.5.0-3-20ed2b9 -prefs=runtime.tools.mkspiffs-2.5.0-3-20ed2b9.path=/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/2.5.0-3-20ed2b9 -prefs=runtime.tools.esptool.path=/home/holla/.arduino15/packages/esp8266/tools/esptool/2.5.0-3-20ed2b9 -prefs=runtime.tools.esptool-2.5.0-3-20ed2b9.path=/home/holla/.arduino15/packages/esp8266/tools/esptool/2.5.0-3-20ed2b9 -prefs=runtime.tools.xtensa-lx106-elf-gcc.path=/home/holla/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/2.5.0-3-20ed2b9 -prefs=runtime.tools.xtensa-lx106-elf-gcc-2.5.0-3-20ed2b9.path=/home/holla/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/2.5.0-3-20ed2b9 -verbose ./$(ino)
	/home/holla/arduino/arduino-builder -compile -logger=machine -hardware /home/holla/arduino/hardware -hardware /home/holla/.arduino15/packages -hardware /home/holla/Arduino/hardware -tools /home/holla/arduino/tools-builder -tools /home/holla/arduino/hardware/tools/avr -tools /home/holla/.arduino15/packages -built-in-libraries /home/holla/arduino/libraries -libraries /home/holla/Arduino/libraries -fqbn=esp8266:esp8266:d1_mini:xtal=80,vt=flash,exception=disabled,eesz=4M3M,ip=lm2f,dbg=Disabled,lvl=None____,wipe=none,baud=921600 -ide-version=10808 -build-path /tmp/arduino_build -warnings=none -build-cache /tmp/arduino_cache -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.mkspiffs.path=/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/2.5.0-3-20ed2b9 -prefs=runtime.tools.mkspiffs-2.5.0-3-20ed2b9.path=/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/2.5.0-3-20ed2b9 -prefs=runtime.tools.esptool.path=/home/holla/.arduino15/packages/esp8266/tools/esptool/2.5.0-3-20ed2b9 -prefs=runtime.tools.esptool-2.5.0-3-20ed2b9.path=/home/holla/.arduino15/packages/esp8266/tools/esptool/2.5.0-3-20ed2b9 -prefs=runtime.tools.xtensa-lx106-elf-gcc.path=/home/holla/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/2.5.0-3-20ed2b9 -prefs=runtime.tools.xtensa-lx106-elf-gcc-2.5.0-3-20ed2b9.path=/home/holla/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/2.5.0-3-20ed2b9 -verbose ./$(ino)

old:
	${arduinodir}/arduino-builder -dump-prefs -logger=machine -hardware ${arduinodir}/hardware -hardware ${arduinosdir15}/packages -hardware ${arduinosketch}/hardware -tools ${arduinodir}/tools-builder -tools ${arduinodir}/hardware/tools/avr -tools ${arduinosdir15}/packages -built-in-libraries ${arduinodir}/libraries -libraries ${arduinosketch}/libraries -fqbn=esp8266:esp8266:d1_mini:CpuFrequency=80,VTable=flash,FlashSize=4M3M,LwIPVariant=v2mss1460,Debug=Disabled,DebugLevel=None____,FlashErase=none,UploadSpeed=921600 -ide-version=10806 -build-path /tmp/arduino_build -warnings=none -build-cache /tmp/arduino_cache -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.mkspiffs.path=${arduinosdir15}/packages/esp8266/tools/mkspiffs/0.2.0 -prefs=runtime.tools.esptool.path=${arduinosdir15}/packages/esp8266/tools/esptool/0.4.13 -prefs=runtime.tools.xtensa-lx106-elf-gcc.path=${arduinosdir15}/packages/esp8266/tools/xtensa-lx106-elf-gcc/2.5.0-3-20ed2b9 -verbose ./$(ino)
	${arduinodir}/arduino-builder -compile -logger=machine -hardware ${arduinodir}/hardware -hardware ${arduinosdir15}/packages -hardware ${arduinosketch}/hardware -tools ${arduinodir}/tools-builder -tools ${arduinodir}/hardware/tools/avr -tools ${arduinosdir15}/packages -built-in-libraries ${arduinodir}/libraries -libraries ${arduinosketch}/libraries -fqbn=esp8266:esp8266:d1_mini:CpuFrequency=80,VTable=flash,FlashSize=4M3M,LwIPVariant=v2mss1460,Debug=Disabled,DebugLevel=None____,FlashErase=none,UploadSpeed=921600 -ide-version=10806 -build-path /tmp/arduino_build -warnings=none -build-cache /tmp/arduino_cache -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.mkspiffs.path=${arduinosdir15}/packages/esp8266/tools/mkspiffs/0.2.0 -prefs=runtime.tools.esptool.path=${arduinosdir15}/packages/esp8266/tools/esptool/0.4.13 -prefs=runtime.tools.xtensa-lx106-elf-gcc.path=${arduinosdir15}/packages/esp8266/tools/xtensa-lx106-elf-gcc/2.5.0-3-20ed2b9 -verbose ./$(ino)

run: bin
	/home/holla/.arduino15/packages/esp8266/tools/esptool/2.5.0-3-20ed2b9/esptool -vv -cd nodemcu -cb 921600 -cp $(port) -ca 0x00000 -cf /tmp/arduino_build/$(ino).bin 

flash: ota

usb: 
	@- pkill -9 -f microcom
	/home/holla/.arduino15/packages/esp8266/tools/esptool/2.5.0-3-20ed2b9/esptool -vv -cd nodemcu -cb 921600 -cp $(port) -ca 0x00000 -cf /tmp/arduino_build/$(ino).bin 
	@- pkill -9 -f sleep

files:
	@echo ${name}
	name=${name} ./transferdata.sh
	curl http://${name}/reload

.PHONY: data

data: 
	@/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/2.5.0-3-20ed2b9/mkspiffs -c data/ -s 0x2FB000 -b 0x2000 /tmp/data.bin
	@/home/holla/.arduino15/packages/esp8266/tools/esptool/2.5.0-3-20ed2b9/esptool -vv -cd nodemcu -cb 921600 -cp /dev/ttyUSB0 -ca 0x100000 -cf /tmp/data.bin

flashall: 
	/home/holla/.arduino15/packages/esp8266/tools/esptool/2.5.0-3-20ed2b9/esptool -vv -cd nodemcu -cb 921600 -cp $(port) -ca 0x00000 -cf /tmp/arduino_build/$(ino).bin 
	sleep 5
	/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/2.5.0-3-20ed2b9/mkspiffs -c data/ -s 0x2FB000 -b 0x2000 /tmp/data.bin
	/home/holla/.arduino15/packages/esp8266/tools/esptool/2.5.0-3-20ed2b9/esptool -vv -cd nodemcu -cb 921600 -cp /dev/ttyUSB0 -ca 0x100000 -cf /tmp/data.bin

reload:
	@echo ${name}
	@echo "reload"
	curl http://${name}/reload

ota:
	@echo ${name}
	@echo "ota"
	curl --progress-bar -F "image=@/tmp/arduino_build/$(ino).bin" http://${name}/upload | tee /dev/null

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

getfiles:
	@echo ${name}
	@echo "downloading index.htm and config.json"
	@wget http://${name}/index.htm -O data/index.htm
	@wget http://${name}/config.json -O data/config.json

clean:
	rm -rf /tmp/arduino_build*
	rm -rf /tmp/arduino_cache*

con:
	@while [ 1 ]; do microcom -p ${port} -s 115200;data;sleep 50000;done

tcp:
	nc ${name} 23

blinkdisable:
	curl http://${name}/heartbeat?enabled=0

blinkenable:
	curl http://${name}/heartbeat?enabled=1
	

erase:
	@- pkill -9 -f microcom
	/home/holla/esptool/esptool.py  -p ${port}  erase_flash

# not quite right
# /home/holla/.arduino15/packages/esp8266/tools/mkspiffs/2.5.0-3-20ed2b9/mkspiffs -c data/ -s 0x2FB000 -b 0x2000 data.bin
# ~/esptool/esptool.py --port /dev/ttyUSB0 write_flash 0x100000 data.bin 
# ~/esptool/esptool.py --port /dev/ttyUSB0 write_flash  0 /tmp/arduino_build/ESP8266Template.ino.bin
# /home/holla/.arduino15/packages/esp8266/tools/esptool/2.5.0-3-20ed2b9/esptool -vv -cd nodemcu -cb 921600 -cp /dev/ttyUSB0 -ca 0x100000 -cf data.bin
