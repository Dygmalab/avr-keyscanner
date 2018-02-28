#pragma once
#include <avr/pgmspace.h>
PROGMEM const uint8_t led_LUT[2][128] = {
#define XXX 0xFF 
    { XXX,    XXX,    XXX,   XXX,   6,   9,   12,  XXX,  XXX,  21,  24,  27,  30,  33,  36,  39,  
      XXX,    XXX,    XXX,   XXX,   7,   10,  13,  XXX,  XXX,  22,  25,  28,  31,  34,  37,  40, 
      XXX,    XXX,    XXX,   XXX,   8,   11,  14,  XXX,  XXX,  23,  26,  29,  32,  35,  38,  41,  
                                                                                                                                      
      42,  45,  48,  XXX,    XXX,    XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  72,  XXX,  XXX,  81,  
      43,  46,  49,  XXX,    XXX,    XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  73,  XXX,  XXX,  82,  
      44,  47,  50,  XXX,    XXX,    XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  74,  XXX,  XXX,  83,  
                                                                                                                                      
      XXX,  XXX,  XXX,  XXX,  96,  XXX,  XXX,    XXX,    XXX, 105, XXX, XXX, XXX, XXX, 120, XXX, 
      XXX,  XXX,  XXX,  XXX,  97,  XXX,  XXX,    XXX,    XXX, 106, XXX, XXX, XXX, XXX, 121, XXX, 
    },                                                                                                                                
    { XXX,  XXX,  XXX,  XXX,  98,  XXX,  XXX,    XXX,    XXX, 107, XXX, XXX, XXX, XXX, 122, XXX, 

      XXX, XXX, XXX, XXX, XXX, XXX, 144, XXX, XXX, XXX,    XXX,    XXX, XXX, XXX, XXX, XXX, 
      XXX, XXX, XXX, XXX, XXX, XXX, 145, XXX, XXX, XXX,    XXX,    XXX, XXX, XXX, XXX, XXX,
      XXX, XXX, XXX, XXX, XXX, XXX, 146, XXX, XXX, XXX,    XXX,    XXX, XXX, XXX, XXX, XXX,
                                                                                                                                      
      XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    
      XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    
      XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    
      XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    XXX,    
  } };
// made a mistake on the pcb and swapped leds l4 and i4, these are starting with 150 and 153, which is why they are reversed above

