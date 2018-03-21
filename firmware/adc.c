#include <stdint.h>
#include <util/delay.h>
#include "main.h"
#include "adc.h"

void setup_adc()
{
    PRR &= ~(1<< PRADC); //write logic one to shutdown, zero to wake

    //ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0); // clock divided by 64 125 kHz
   // ADCSRA |= (1 << ADATE); // setting auto trigger
//  ADCSRB |= (0 << ADTS2) | (0 << ADTS1) | (0 << ADTS0); // free running mode - default
//  ADMUX |= (0 << REFS0); // ref voltage is Vcc - default
//  ADMUX |= (0 << ADLAR); // using 10 bit resolution - default

    // I want adc7 - pa1 on matrix board mux[3:0] - 0111
    ADMUX &= ~(1 << MUX3);
    ADMUX |=  (1 << MUX2);
    ADMUX |=  (1 << MUX1);
    ADMUX |=  (1 << MUX0);

    _delay_us(200); // wait for 25 ADC cycles at 125kHz

    ADCSRA |= (1 << ADEN); // enable ADC
    ADCSRA |= (1 << ADSC);  // start sampling free running mode
}


uint16_t read_adc()
{
    uint8_t low = ADCL;
    uint8_t high = ADCH;
    uint16_t result = high;
    result = result << 8;
    result += low;
    return result;
}

