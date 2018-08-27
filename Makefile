ino = $(wildcard *.ino)
arduinopath = ~/arduino

bin:
	@mkdir -p /tmp/arduino_build /tmp/arduino_cache
	$(arduinopath)/arduino-builder -dump-prefs -logger=machine -hardware $(arduinopath)/hardware -hardware /home/holla/.arduino15/packages -hardware /home/holla/Arduino/hardware -tools $(arduinopath)/tools-builder -tools $(arduinopath)/hardware/tools/avr -tools /home/holla/.arduino15/packages -built-in-libraries $(arduinopath)/libraries -libraries /home/holla/Arduino/libraries -fqbn=esp8266:esp8266:d1:CpuFrequency=80,VTable=flash,FlashSize=4M1M,LwIPVariant=v2mss536,Debug=Disabled,DebugLevel=None____,FlashErase=none,UploadSpeed=921600 -ide-version=10805 -build-path /tmp/arduino_build -warnings=none -build-cache /tmp/arduino_cache -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.mkspiffs.path=/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/0.2.0 -prefs=runtime.tools.esptool.path=/home/holla/.arduino15/packages/esp8266/tools/esptool/0.4.13 -prefs=runtime.tools.xtensa-lx106-elf-gcc.path=/home/holla/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/1.20.0-26-gb404fb9-2 -verbose $(ino)
	$(arduinopath)/arduino-builder -compile -logger=machine -hardware $(arduinopath)/hardware -hardware /home/holla/.arduino15/packages -hardware /home/holla/Arduino/hardware -tools $(arduinopath)/tools-builder -tools $(arduinopath)/hardware/tools/avr -tools /home/holla/.arduino15/packages -built-in-libraries $(arduinopath)/libraries -libraries /home/holla/Arduino/libraries -fqbn=esp8266:esp8266:d1:CpuFrequency=80,VTable=flash,FlashSize=4M1M,LwIPVariant=v2mss536,Debug=Disabled,DebugLevel=None____,FlashErase=none,UploadSpeed=921600 -ide-version=10805 -build-path /tmp/arduino_build -warnings=none -build-cache /tmp/arduino_cache -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.mkspiffs.path=/home/holla/.arduino15/packages/esp8266/tools/mkspiffs/0.2.0 -prefs=runtime.tools.esptool.path=/home/holla/.arduino15/packages/esp8266/tools/esptool/0.4.13 -prefs=runtime.tools.xtensa-lx106-elf-gcc.path=/home/holla/.arduino15/packages/esp8266/tools/xtensa-lx106-elf-gcc/1.20.0-26-gb404fb9-2 -verbose $(ino)

run: bin
	/home/holla/.arduino15/packages/esp8266/tools/esptool/0.4.13/esptool -vv -cd nodemcu -cb 921600 -cp /dev/ttyUSB0 -ca 0x00000 -cf /tmp/arduino_build/$(ino).bin 

flash: 
	/home/holla/.arduino15/packages/esp8266/tools/esptool/0.4.13/esptool -vv -cd nodemcu -cb 921600 -cp /dev/ttyUSB0 -ca 0x00000 -cf /tmp/arduino_build/$(ino).bin 

clean:
	rm -rf /tmp/arduino_build/*
	rm -rf /tmp/arduino_cache/*
