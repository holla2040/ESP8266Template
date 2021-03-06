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

espversion = $(shell find ~/.arduino15/ -name xtensa-lx106-elf-g++ | cut -f 9 -d '/')

spiffs=4M2M

try:
	@mkdir -p /tmp/arduino_build /tmp/arduino_cache
#	/home/holla/arduino/arduino-builder -compile -logger=machine -hardware /home/holla/arduino/hardware -hardware /home/holla/.arduino15/packages -hardware /home/holla/Arduino/hardware -tools /home/holla/arduino/tools-builder -tools /home/holla/arduino/hardware/tools/avr -tools /home/holla/.arduino15/packages -built-in-libraries /home/holla/arduino/libraries -libraries /home/holla/Arduino/libraries -fqbn=esp8266:esp8266:d1_mini:xtal=80,vt=flash,exception=disabled,eesz=$(spiffs),ip=lm2f,dbg=Disabled,lvl=None____,wipe=none,baud=921600 -ide-version=10808 -build-path /tmp/arduino_build -warnings=none -build-cache /tmp/arduino_cache -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.mkspiffs.path=/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/$(espversion) -prefs=runtime.tools.mkspiffs-$(espversion).path=/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/$(espversion) -prefs=runtime.tools.esptool.path=/home/holla/.arduino15/packages/esp8266/tools/esptool/$(espversion) -prefs=runtime.tools.esptool-$(espversion).path=/home/holla/.arduino15/packages/esp8266/tools/esptool/$(espversion) -prefs=runtime.tools.xtensa-lx106-elf-gcc.path=/home/holla/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/$(espversion) -prefs=runtime.tools.xtensa-lx106-elf-gcc-$(espversion).path=/home/holla/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/$(espversion) -verbose ./$(ino)
	/mnt/ramdisk/arduino/arduino-1.8.10/arduino-builder -compile -logger=machine -hardware /mnt/ramdisk/arduino/arduino-1.8.10/hardware -hardware /home/holla/.arduino15/packages -hardware /home/holla/Arduino/hardware -tools /mnt/ramdisk/arduino/arduino-1.8.10/tools-builder -tools /mnt/ramdisk/arduino/arduino-1.8.10/hardware/tools/avr -tools /home/holla/.arduino15/packages -built-in-libraries /mnt/ramdisk/arduino/arduino-1.8.10/libraries -libraries /home/holla/Arduino/libraries -fqbn=esp8266:esp8266:d1_mini:xtal=80,vt=flash,exception=legacy,ssl=all,eesz=4M2M,ip=hb2f,dbg=Disabled,lvl=None____,wipe=none,baud=921600 -ide-version=10810 -build-path /tmp/arduino_build -warnings=none -build-cache /tmp/arduino_cache -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.mklittlefs.path=/home/holla/.arduino15/packages/esp8266/tools/mklittlefs/2.5.0-4-69bd9e6 -prefs=runtime.tools.mklittlefs-2.5.0-4-69bd9e6.path=/home/holla/.arduino15/packages/esp8266/tools/mklittlefs/2.5.0-4-69bd9e6 -prefs=runtime.tools.python3.path=/home/holla/.arduino15/packages/esp8266/tools/python3/3.7.2-post1 -prefs=runtime.tools.python3-3.7.2-post1.path=/home/holla/.arduino15/packages/esp8266/tools/python3/3.7.2-post1 -prefs=runtime.tools.mkspiffs.path=/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/2.5.0-4-b40a506 -prefs=runtime.tools.mkspiffs-2.5.0-4-b40a506.path=/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/2.5.0-4-b40a506 -prefs=runtime.tools.xtensa-lx106-elf-gcc.path=/home/holla/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/2.5.0-4-b40a506 -prefs=runtime.tools.xtensa-lx106-elf-gcc-2.5.0-4-b40a506.path=/home/holla/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/2.5.0-4-b40a506 -verbose /home/holla/ESP8266Template/ESP8266Template.ino



run: bin
	/home/holla/.arduino15/packages/esp8266/tools/esptool/$(espversion)/esptool -vv -cd nodemcu -cb 921600 -cp $(port) -ca 0x00000 -cf /tmp/arduino_build/$(ino).bin 

flash: ota

usb: 
	@- pkill -9 -f microcom
	esptool -vv -cd nodemcu -cb 921600 -cp $(port) -ca 0x00000 -cf /tmp/arduino_build/$(ino).bin 
	@- pkill -9 -f sleep

files:
	@echo ${name}
	name=${name} ./transferdata.sh
	curl http://${name}/reload

.PHONY: data

data3m: 
	@/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/$(espversion)/mkspiffs -c data/ -s 0x2FB000 -b 0x2000 /tmp/data.bin
	@esptool -vv -cd nodemcu -cb 921600 -cp $(port) -ca 0x100000 -cf /tmp/data.bin

data2m: 
	@/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/$(espversion)/mkspiffs -c data/ -s 0x1FB000 -b 0x2000 /tmp/data.bin
	@@esptool -vv -cd nodemcu -cb 921600 -cp $(port) -ca 0x200000 -cf /tmp/data.bin

data1m: 
	@/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/$(espversion)/mkspiffs -c data/ -s 0xFB000 -b 0x2000 /tmp/data.bin
	@esptool -vv -cd nodemcu -cb 921600 -cp $(port) -ca 0x300000 -cf /tmp/data.bin

flashall: 
	esptool -vv -cd nodemcu -cb 921600 -cp $(port) -ca 0x00000 -cf /tmp/arduino_build/$(ino).bin 
	sleep 5
	/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/$(espversion)/mkspiffs -c data/ -s 0x2FB000 -b 0x2000 /tmp/data.bin
	esptool -vv -cd nodemcu -cb 921600 -cp $(port) -ca $(spibaseaddr) -cf /tmp/data.bin

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

