default: all

# Name: Makefile
# Author: Scott Perry <dev@numist.net>
# Copyright: 2015
# License: MIT

# You should at least check the settings for
# DEVICE ....... The AVR device you compile for
# CLOCK ........ Target AVR clock rate in Hertz
# PROGRAMMER ... Options to avrdude which define the hardware you use for
#                uploading to the AVR and the interface where this hardware
#                is connected. We recommend that you leave it undefined and
#                add settings like this to your ~/.avrduderc file:
#                   default_programmer = "dragon_isp";
#                   default_serial = "usb";
# FUSES ........ Parameters for avrdude to flash the fuses appropriately.

DEVICE     ?= attiny88

# 8Mhz
CLOCK      ?= 8000000

# For computing fuse byte values for other devices and options see
# the fuse bit calculator at http://www.engbedded.com/fusecalc/
FUSES      = -U lfuse:w:0xee:m -U hfuse:w:0xdd:m -U efuse:w:0xfe:m

AVRDUDE_PATH ?= avrdude
GCC_PATH ?= avr-gcc
PROGRAMMER ?= -c dragon_isp

AVRDUDE = $(AVRDUDE_PATH) $(PROGRAMMER) -p $(DEVICE) -v

flash:	all
	#$(AVRDUDE) -B 100 -U flash:w:out/attiny88_factory.hex:i
	echo $(AVRDUDE) -B 1 -U flash:w:out/attiny88_keyscanner.hex:i
	$(AVRDUDE) -B 1 -U flash:w:out/attiny88_keyscanner.hex:i

fuse:
	$(AVRDUDE) -B 100 $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install: all fuse flash

clean:
	rm -fr out/*

all: build flashing-tool

build:
	mkdir -p out
	
	firmware/gen_map.py left > firmware/map.h
	make -C firmware clean all
	cp firmware/main.hex out/attiny88_keyscanner_left.hex
	
	firmware/gen_map.py right > firmware/map.h
	make -C firmware clean all
	cp firmware/main.hex out/attiny88_keyscanner_right.hex
	
	# stitch bootloader and hex into one file for programming
	./tools/make_factory_firmware.py out/attiny88_keyscanner_left.hex out/factory_left.hex
	./tools/make_factory_firmware.py out/attiny88_keyscanner_right.hex out/factory_right.hex

flashing-tool: build
	mkdir -p out/attiny_flasher_left
	mkdir -p out/attiny_flasher_right
	cp etc/flasher_Makefile out/attiny_flasher_left/Makefile
	cp etc/flasher_Makefile out/attiny_flasher_right/Makefile
	# make a flashing firmware for the huble to flash the left and right sides
	python2.7 ./tools/hex_to_atmega.py out/attiny88_keyscanner_left.hex > out/attiny_flasher_left/attiny_flasher.ino
	python2.7 ./tools/hex_to_atmega.py out/attiny88_keyscanner_right.hex > out/attiny_flasher_right/attiny_flasher.ino


.PHONY: default all clean install flash fuse
