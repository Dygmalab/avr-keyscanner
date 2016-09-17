#!/usr/bin/env python2.7

from collections import namedtuple
import sys

page_size = 64
frame_size = 16
memory_size = 8192
blank = 0xff
delay_ms = 1

template = '''
#include "Arduino.h"
#include "Wire.h"

void setup() {
  Wire.begin();
}

#define page_size %d
#define frame_size %d
#define blank 0x%x
#define pages %d
#define firmware_length %d
#define DELAY %d
uint16_t offsets[pages] = {%s};
byte firmware[firmware_length] = {%s};

byte written = 0;

void loop() {
  if (written != 0) {
    // we're done
    return;
  }

  Serial.print("Communicating\\n");
  byte addr = 0x58;

  byte result = 2;
  while (result != 3) {
    Serial.print("Erasing\\n");
    Wire.beginTransmission(addr);
    Wire.write(0x04); // erase user space
    Wire.write(0x00); // dummy end byte
    result = Wire.endTransmission();
    Serial.print("result = ");
    Serial.print(result);
    Serial.print("\\n");

    _delay_ms(100);
  }

  byte o = 0;

  for (uint16_t i = 0; i < firmware_length; i += page_size) {
    Serial.print("Setting addr\\n");
    Wire.beginTransmission(addr);
    Wire.write(0x1); // write page addr
    Wire.write(offsets[o] & 0xff); // write page addr
    Wire.write(offsets[o] >> 8);
    Wire.write(0x00); // dummy end byte
    result = Wire.endTransmission();
    Serial.print("result = ");
    Serial.print(result);
    Serial.print("\\n");
    _delay_ms(DELAY);
    // got something other than NACK. Start over.
    if (result != 3) {
      return;
    }

    // transmit each frame separately
    for (uint8_t frame = 0; frame < page_size / frame_size; frame++) {
      Wire.beginTransmission(addr);
      Wire.write(0x2); // continue page
      for (uint8_t j = frame * frame_size; j < (frame + 1) * frame_size; j++) {
        if (i + j < firmware_length) {
          Wire.write(firmware[i + j]);
        } else {
          Wire.write(blank);
        }
      }
      Wire.write(0x00); // dummy end byte
      result = Wire.endTransmission();
      Serial.print("got ");
      Serial.print(result);
      Serial.print(" for page ");
      Serial.print(offsets[o]);
      Serial.print(" frame ");
      Serial.print(frame);
      Serial.print("\\n");
      // got something other than NACK. Start over.
      if (result != 3) {
        return;
      }
      delay(DELAY);
    }
    o++;
  }
  written = 1; // firmware successfully rewritten

  Serial.print("resetting\\n");
  Wire.beginTransmission(addr);
  Wire.write(0x03); // execute app
  Wire.write(0x00); // dummy end byte
  result = Wire.endTransmission();
  Serial.print("result ");
  Serial.print(result);
  Serial.print("\\n");

  Serial.print("done\\n");
}
'''

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

with open(sys.argv[1]) as fin:
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

print template % (page_size, frame_size, blank, len(offsets), len(data), delay_ms, offsets_text, data_text)
