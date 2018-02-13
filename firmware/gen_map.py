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

# mapping from pcb to led number
left_key_map = [ 5, 6, 7, 8, 9, 10, 11, 22, 23, 24, 25, 26, 27, 34, 35, 36, 37, 38, 39, 42, 43, 44, 45, 46, 47, 50, 51, 52, 53, 54, 62, 63] 
left_underglow_map = [ 56, 58, 49, 41, 59, 21, 33, 20, 19, 13, 57, 12, 32, 40, 48, 55, 60, 61] # last 2 are lp unders
left_palm_map = [ 0, 1, 2, 3, 4, 14, 15, 16, 17, 18, 28, 29, 30, 31 ]

# list of leds that are reverse mounted (different wiring of rgb)
left_led_rev = left_palm_map + [56, 55, 62, 63 ]

# mapping from pcb to led number
#right_key_map = [ 5, 6, 7, 8, 9, 10, 11, 60, 22, 23, 24, 25, 26, 27, 61, 36, 37, 38, 39, 40, 41, 62, 44, 45, 46, 47, 48, 49, 52, 53, 54, 55, 56, 65, 66] 
right_key_map = [ 11, 10, 9, 8, 7, 6, 5, 61, 27, 26, 25, 24, 23, 22, 60, 62, 41, 40, 39, 38, 37, 36, 49, 48, 47, 46, 45, 44, 56, 55, 54, 53, 52, 63, 66, 65] 
right_underglow_map = [ 57, 58, 35, 43, 21, 67, 20, 19,13, 12, 59, 69, 34, 42, 68, 50, 51, 64] # last 1 are lp unders - removed one for the missing key
right_palm_map = [ 0, 1, 2, 3, 4, 14, 15, 16, 17, 18, 28, 29, 30, 31, 32, 33 ]

# list of leds that are reverse mounted (different wiring of rgb)
right_led_rev = right_palm_map + [57, 51, 65, 66 ]


if right:
    key_map = right_key_map
    underglow_map = right_underglow_map
    palm_map = right_palm_map
    led_rev = right_led_rev

    n_palms = 16
    n_unders = 17 
    n_unders_lps = 1
    n_keys = 34 # should be 34 but led h4 got moved to lps...
    n_keys_lps = 2
elif left:
    key_map = left_key_map
    underglow_map = left_underglow_map
    palm_map = left_palm_map
    led_rev = left_led_rev

    n_palms = 14
    n_unders = 16
    n_unders_lps = 2
    n_keys = 30
    n_keys_lps = 2

##############################
# don't change below here

all_map = key_map + underglow_map + palm_map
num_rgbs = len(all_map)

#for i in range(32):
#    print(all_map[i])

# couple of assertions on valid input


num_rgbs = n_keys_lps + n_keys + n_unders_lps + n_unders + n_palms
print("//generating map for %d rgbs (%d leds)" % (num_rgbs, num_rgbs*3))

for i in range(num_rgbs):
    assert i in all_map, "%d not in map" % i

assert len(key_map) == n_keys + n_keys_lps, "len keymap %d != num keys %d" % (len(key_map), n_keys + n_keys_lps)
assert len(underglow_map) == n_unders + n_unders_lps, "len under %d != unders %d" % (len(underglow_map), n_unders + n_unders_lps)
assert len(palm_map) == n_palms


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

if left:
    # missing leds due to deletions on the pcbs
    x = 4
    y = 6
    led_mask[y:y+3, x:x+1] = [[0],[0],[0]]
    x = 5
    led_mask[y:y+3, x:x+1] = [[0],[0],[0]]
    x = 6
    y = 12
    led_mask[y:y+3, x:x+1] = [[0],[0],[0]]

#print(led_mask)

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
print("""// generated by gen_map.py - for %s side
#pragma once
#include <avr/pgmspace.h>
#define XXX 0xFF
PROGMEM const uint8_t led_LUT[2][128] = {
   {""" % (sys.argv[1]))

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