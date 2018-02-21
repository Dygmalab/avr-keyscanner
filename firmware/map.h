#pragma once
#include <avr/pgmspace.h>
PROGMEM const uint8_t led_LUT[2][128] = {
#define XXX 0xFF 
    { XXX,    XXX,    1,   4,   6,   9,   12,  16,  19,  21,  24,  27,  30,  33,  36,  39,  
      XXX,    XXX,    0,   3,   7,   10,  13,  15,  18,  22,  25,  28,  31,  34,  37,  40, 
      XXX,    XXX,    2,   5,   8,   11,  14,  17,  20,  23,  26,  29,  32,  35,  38,  41,  
                                                                                                                                      
      42,  45,  48,  XXX,    XXX,    51,  54,  57,  60,  63,  66,  69,  72,  75,  78,  81,  
      43,  46,  49,  XXX,    XXX,    52,  55,  58,  61,  64,  67,  70,  73,  76,  79,  82,  
      44,  47,  50,  XXX,    XXX,    53,  56,  59,  62,  65,  68,  71,  74,  77,  80,  83,  
                                                                                                                                      
      84,  87,  90,  93,  96,  99,  XXX,    XXX,    102, 105, 108, 111, 114, 117, 120, 123, 
      85,  88,  91,  94,  97,  100, XXX,    XXX,    103, 106, 109, 112, 115, 118, 121, 124, 
    },                                                                                                                                
    { 86,  89,  92,  95,  98,  101, XXX,    XXX,    104, 107, 110, 113, 116, 119, 122, 125, 

      126, 129, 132, 135, 138, 141, 144, 147, 153, XXX,    XXX,    150, 156, 159, 162, 165, 
      127, 130, 133, 136, 139, 142, 145, 148, 154, XXX,    XXX,    151, 157, 160, 163, 166,
      128, 131, 134, 137, 140, 143, 146, 149, 155, XXX,    XXX,    152, 158, 161, 164, 167,
                                                                                                                                      
      XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    
      XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    
      XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    
      XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    
  } };
// made a mistake on the pcb and swapped leds l4 and i4, these are starting with 150 and 153, which is why they are reversed above

