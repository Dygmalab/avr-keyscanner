# Name: Makefile
# Author: Scott Perry <dev@numist.net>
# Copyright: 2015
# License: MIT

# You should at least check the settings for
# DEVICE ....... The AVR device you compile for
# CLOCK ........ Target AVR clock rate in Hertz
# OBJECTS ...... The object files created from your source files. This list is
#                usually the same as the list of source files with suffix ".o".
# PROGRAMMER ... Options to avrdude which define the hardware you use for
#                uploading to the AVR and the interface where this hardware
#                is connected. We recommend that you leave it undefined and
#                add settings like this to your ~/.avrduderc file:
#                   default_programmer = "dragon_isp";
#                   default_serial = "usb";
# FUSES ........ Parameters for avrdude to flash the fuses appropriately.

# To override any of these for your local setup, create a file Makefile.local
# (ignored by git)
-include Makefile.local

# See avr-help for all possible devices
DEVICE     ?= attiny88

# 8Mhz
CLOCK      ?= 8000000

# Add something like this to Makefile.local if you haven't yet set up ~/.avrduderc with a default programmer.
# PROGRAMMER = -c usbtiny -P usb
#PROGRAMMER = -c dragon_isp 
PROGRAMMER = -c stk500v2 -P /dev/ttyACM2

# Add more objects for each .c file here
OBJECTS    = main.o sled1735.o wire-protocol.o twi-slave.o ringbuf.o keyscanner.o

# For computing fuse byte values for other devices and options see
# the fuse bit calculator at http://www.engbedded.com/fusecalc/
FUSES      = -U lfuse:w:0xee:m -U hfuse:w:0xdd:m -U efuse:w:0xfe:m
FUSESDEBUG = -U lfuse:w:0xee:m -U hfuse:w:0x9d:m -U efuse:w:0xfe:m 

AVRDUDE_PATH ?= avrdude
GCC_PATH ?= avr-gcc
# Tune the lines below only if you know what you are doing:

# Optimize for many things (including perf)
OPTIMIZATION = -O2
#DEBUG = -ggdb

# Optimize, but focus on keeping code size small
OPTIMIZATION = -Os

AVRDUDE = $(AVRDUDE_PATH) $(PROGRAMMER) -p $(DEVICE)
COMPILE = $(GCC_PATH) -Wall -Wextra $(OPTIMIZATION) $(DEBUG) -std=gnu11 -flto -mmcu=$(DEVICE) -DF_CPU=$(CLOCK)

# symbolic targets:
all:	main.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -v -B 1 -U flash:w:main.hex:i

fuse:
	$(AVRDUDE) -B 100 $(FUSES)

fuse-debug:
	$(AVRDUDE) -B 100 $(FUSESDEBUG)

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID main.hex

clean:
	rm -f main.hex main.elf $(OBJECTS)

# file targets:
main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size --format=avr --mcu=$(DEVICE) main.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c

astyle:
	find . -type f -name \*.c |xargs -n 1 astyle --style=google
	find . -type f -name \*.h |xargs -n 1 astyle --style=google
