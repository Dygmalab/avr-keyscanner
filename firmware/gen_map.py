#!/usr/bin/python

import numpy as np

sled_rows = 16
sled_cols = 16
LUT_banks = 2

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
# quantity of RGBS to distribute
num_rgbs = 60

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
    led_map[y:y+3, x:x+1] = [[led_num * 3],[led_num * 3 + 1],[led_num * 3 + 2]]


# format it for c
print("{\n    {")

for y in range(sled_rows):
    if y == sled_rows / LUT_banks:
        print("    },\n    {")
    line = "        "
    for x in range(sled_cols):
        line += "%3d, " % led_map[y][x]
    print(line)
print("    }\n};")
