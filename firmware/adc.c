#include <stdint.h>
#include <util/delay.h>
#include "main.h"
#include "adc.h"

uint16_t read_adc(uint8_t mux)
{
    PRR &= ~(1<< PRADC); //write logic one to shutdown, zero to wake
    ADMUX =
        (0 << ADLAR) |     // don't left shift result
        (1 << REFS0) |     // Sets ref. voltage to avcc
        (mux & 0x0F);
        /*
        (0 << MUX3)  |     // MUX[3:0] = 0110 = 6 = ADC6 = PA0
        (1 << MUX2)  |
        (1 << MUX1)  |
        (1 << MUX0);
        */

    ADCSRA = 
        (1 << ADEN)  |     // Enable ADC 
        (1 << ADPS2) |     // set prescaler to 64, bit 2 
        (1 << ADPS1) |     // set prescaler to 64, bit 1 
        (0 << ADPS0);      // set prescaler to 64, bit 0  

    ADCSRA |= (1 << ADSC);         // start ADC measurement
    while (ADCSRA & (1 << ADSC) ); // wait till conversion complete 
    uint8_t low = ADCL;
    uint8_t high = ADCH;
    uint16_t result = high;
    result = result << 8;
    result += low;
    return result;
}
