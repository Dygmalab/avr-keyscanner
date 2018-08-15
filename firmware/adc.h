#pragma once

uint16_t read_adc(uint8_t mux);

#define ADC_CC 6    // ADC6
#define ADC_HALL 7  //ADC7

// hall output is at 2.3v normally, varies between 1.8 and 3v.
// this is 370 -> 615 ADC counts. So subtract offset to get a number
// that will fit in a byte
#define ADC_HALL_OFFSET 350
