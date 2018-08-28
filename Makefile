# copy 2 arduino IDE arduino-builder lines to bin
# delete the unique numbers from build and cache directory, these are unique to IDE runs
# :s/_69751//g
# :s/_90469//g
# if you want quieter output, 
#  delete -dump-prefs -verbose
#  add -quiet=1
# hopefully arduino-cli will eliminate this hack

ino = $(wildcard *.ino)

bin:
	@mkdir -p /tmp/arduino_build /tmp/arduino_cache
	/mnt/ramdisk/arduino/arduino-1.8.6/arduino-builder -quiet=1 -logger=machine -hardware /mnt/ramdisk/arduino/arduino-1.8.6/hardware -hardware /home/holla/.arduino15/packages -hardware /home/holla/Arduino/hardware -tools /mnt/ramdisk/arduino/arduino-1.8.6/tools-builder -tools /mnt/ramdisk/arduino/arduino-1.8.6/hardware/tools/avr -tools /home/holla/.arduino15/packages -built-in-libraries /mnt/ramdisk/arduino/arduino-1.8.6/libraries -libraries /home/holla/Arduino/libraries -fqbn=esp8266:esp8266:d1_mini:CpuFrequency=80,VTable=flash,FlashSize=4M1M,LwIPVariant=v2mss536,Debug=Disabled,DebugLevel=None____,FlashErase=none,UploadSpeed=921600 -ide-version=10806 -build-path /tmp/arduino_build -warnings=none -build-cache /tmp/arduino_cache -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.mkspiffs.path=/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/0.2.0 -prefs=runtime.tools.xtensa-lx106-elf-gcc.path=/home/holla/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/1.20.0-26-gb404fb9-2 -prefs=runtime.tools.esptool.path=/home/holla/.arduino15/packages/esp8266/tools/esptool/0.4.13 $(ino)
	/mnt/ramdisk/arduino/arduino-1.8.6/arduino-builder -quiet=1 -compile -logger=machine -hardware /mnt/ramdisk/arduino/arduino-1.8.6/hardware -hardware /home/holla/.arduino15/packages -hardware /home/holla/Arduino/hardware -tools /mnt/ramdisk/arduino/arduino-1.8.6/tools-builder -tools /mnt/ramdisk/arduino/arduino-1.8.6/hardware/tools/avr -tools /home/holla/.arduino15/packages -built-in-libraries /mnt/ramdisk/arduino/arduino-1.8.6/libraries -libraries /home/holla/Arduino/libraries -fqbn=esp8266:esp8266:d1_mini:CpuFrequency=80,VTable=flash,FlashSize=4M1M,LwIPVariant=v2mss536,Debug=Disabled,DebugLevel=None____,FlashErase=none,UploadSpeed=921600 -ide-version=10806 -build-path /tmp/arduino_build -warnings=none -build-cache /tmp/arduino_cache -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.mkspiffs.path=/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/0.2.0 -prefs=runtime.tools.xtensa-lx106-elf-gcc.path=/home/holla/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/1.20.0-26-gb404fb9-2 -prefs=runtime.tools.esptool.path=/home/holla/.arduino15/packages/esp8266/tools/esptool/0.4.13 $(ino)


run: bin
	/home/holla/.arduino15/packages/esp8266/tools/esptool/0.4.13/esptool -vv -cd nodemcu -cb 921600 -cp /dev/ttyUSB0 -ca 0x00000 -cf /tmp/arduino_build/$(ino).bin 

flash: 
	/home/holla/.arduino15/packages/esp8266/tools/esptool/0.4.13/esptool -vv -cd nodemcu -cb 921600 -cp /dev/ttyUSB0 -ca 0x00000 -cf /tmp/arduino_build/$(ino).bin 

files:
	@./transferdata.sh

index:
	@echo "uploading index.htm"
	@curl -F "file=@./data/index.htm" myLoc.local/edit

clean:
	rm -rf /tmp/arduino_build/*
	rm -rf /tmp/arduino_cache/*
