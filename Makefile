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
PROGRAMMER ?= -c usbtiny

AVRDUDE = $(AVRDUDE_PATH) $(PROGRAMMER) -p $(DEVICE) -v

flash:	all
	$(AVRDUDE) -B 2 -U flash:w:attiny88_factory.hex:i

fuse:
	$(AVRDUDE) -B 100 $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install: all fuse flash

clean:
	rm -f attiny88_factory.hex

all:
	make -C firmware
	./tools/make_factory_firmware.py

flashing-tool:
	mkdir -p out/flasher
	make -C firmware
	python2.7 ./tools/hex_to_atmega.py firmware/main.hex > out/flasher/flasher.ino


.PHONY: default all clean install flash fuse
