#!/usr/bin/env python2.7

from collections import namedtuple
import sys

page_size = 64
frame_size = 16
memory_size = 8192
blank = 0xff
delay_ms = 1

template = open('etc/flash_firmware_ino.template', 'r').read()

Line = namedtuple('Line', ['size', 'offset', 'kind', 'data', 'checksum'])

def parse_line(line):
    """Parses an Intel HEX line into a Line tuple"""
    if len(line) < 11:
        exit("invalid Intel HEX line: %s" % line)
    if line[0] != ':':
        exit("invalid Intel HEX line: %s" % line)
    line = line[1:]
    try:
        int(line, 16)
    except ValueError:
        exit("invalid Intel HEX line: %s" % line)
    line = line.decode('hex')
    return Line(line[0], line[1:3], line[3], line[4:-1], line[-1])

def read_hex(filename):

    with open(filename) as fin:
        hex_in = fin.read()

    hex_lines = [parse_line(l) for l in hex_in.strip().split()]
    hex_lines = [l for l in hex_lines if l.kind == '\x00']

    mem = [blank] * memory_size
    offsets = []
    data = []
    for line in hex_lines:
        offset = (ord(line.offset[0]) << 8) + ord(line.offset[1])
        for i, x in enumerate(line.data):
            mem[offset + i] = ord(x)

# # if the first offset is not 0, then we need to write an additional 4 bytes for some reason
# for i in xrange(0, len(mem)):
#     if mem[i] != 0:
#         if i >= 4:
#             mem[i-4] = blank
#             mem[i-3] = blank
#             mem[i-2] = blank
#             mem[i-1] = blank
#         break
#
# scan memory
    for i in xrange(0, len(mem), page_size):
        # can skip this page
        if all(x == blank for x in mem[i:i+page_size]):
            continue
        offsets.append(i)
        data.extend(mem[i:i+page_size])

    offsets_text = ', '.join(str(x) for x in offsets)
    data_text = ', '.join(hex(x) for x in data)

    return offsets, data, offsets_text, data_text

try:
    # only difference between the 2 is the data_text
    offsets, data, offsets_text, data_text_left =  read_hex(sys.argv[1])
    offsets, data, offsets_text, data_text_right = read_hex(sys.argv[2])
except IndexError as e:
    exit("must provide both left and right hex files")

print template % (page_size, frame_size, blank, len(offsets), len(data), delay_ms, offsets_text, data_text_left, data_text_right)
