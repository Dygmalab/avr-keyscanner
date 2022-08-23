#pragma once

uint16_t read_adc(uint8_t mux);
uint16_t middle_of_3(uint16_t a, uint16_t b, uint16_t c);

#define ADC_CC 6   // ADC6
#define ADC_HALL 7 // ADC7
