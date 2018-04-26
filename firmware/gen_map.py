#!/usr/bin/python
import numpy as np
import sys

left = False
right = False
if len(sys.argv) != 2:
    print("must specify left or right")
    exit(1)
if sys.argv[1] == 'left':
    left = True
    address = 0x58
elif sys.argv[1] == 'right':
    right = True
    address = 0x5B
else:
    print("unknown arg")


# hard definitions
sled_rows = 16
sled_cols = 16
LUT_banks = 2

num_rgbs = 70 # max for sled1735 with common anode
all_map = range(num_rgbs)

# reverse leds for low profile
led_rev = [] #[ 60, 61 ]
print("//generating map for %d rgbs (%d leds)" % (num_rgbs, num_rgbs*3))

# sled driver has blank patches distributed through the matrix that can't be used for leds
# make a mask to define where these places are
led_mask = np.ones((sled_rows,sled_cols),dtype=int)
x = 5
y = 0
for x in range(0, 13, 3):
    led_mask[y:y+3, x:x+2] = [[0,0],[0,0],[0,0]]
    y += 3

# last row is all blank
led_mask[sled_rows-1] = [0] * sled_cols

# now generate the lookup table
led_map = np.zeros((sled_rows,sled_cols),dtype=int)
led_map.fill(-1)

# distribute the leds through the matrix, avoiding blank parts of the mask
x = 0
y = 0
for led_num in range(num_rgbs):
    # check if can place on the map
    while True:
        x += 1
        if x == sled_cols:
            y += 3
            x = 0
        if led_mask[y][x] == 1:
            break
    # place it
    led_pos = all_map.index(led_num)
    #led_pos = all_map[led_num]
    # each led has its RGB legs tied to subsequent rows
    if led_num in led_rev:
        led_map[y:y+3, x:x+1] = [[led_pos * 3],[led_pos * 3 + 1],[led_pos * 3 + 2]]
    else: # swap g & b
        led_map[y:y+3, x:x+1] = [[led_pos * 3],[led_pos * 3 + 2],[led_pos * 3 + 1]]


# format it for c
print("""// generated by gen_map.py
#pragma once
#include <avr/pgmspace.h>
#define XXX 0xFF
PROGMEM const uint8_t led_LUT[2][128] = {
   {""")

for y in range(sled_rows):
    if y == sled_rows / LUT_banks:
        print("    },\n    {")
    line = "        "
    for x in range(sled_cols):
        if led_map[y][x] != -1:
            line += "%3d, " % led_map[y][x]
        else:
            line += "XXX, "
    print(line)
print("    }\n};")

with open("i2c_addr.h", 'w') as fh:
    fh.write("// generated by gen_map.py - for %s side\n#define I2C_ADDRESS 0x%x\n" % (sys.argv[1], address))

