#include "sled1735.h"
#include <util/delay.h>
#include "main.h"
#include "keyscanner.h"
#include "wire-protocol.h"
#include <stdint.h>

#define TEST_P 3

//#define DETECT_PIN // tested with 80k pulldown on port
#define DETECT_ADC


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

uint8_t red[3] = { 255, 0, 0 };
uint8_t grn[3] = { 0, 255, 0 };
uint8_t off[3] = { 0, 0, 0 };

static inline void setup(void) {
    setup_spi(); // setup sled 1735 driver chip

    keyscanner_init();

    #ifdef DETECT_PIN

    DDRA &= ~_BV(1); // setup as input
    PORTA &= ~_BV(1); // pull down
    _delay_ms(10); // wait for chip to be ready

    // if getting signal over usb C that pc is connected
    while(PINA & _BV(1))
        led_set_all_to( red );

    #endif
    #ifdef DETECT_ADC

    setup_adc();
    while(true)
    {
        if(read_adc() > 500)
            led_set_all_to( red );
        else
            led_set_all_to( grn );
    }
    #endif
    led_set_all_to( red );
    //twi_init();
}
#define R 0
#define G 50
#define B 200
int main(void) {
    setup();

    while(1) {

    //    keyscanner_main();
    led_set_all_to( red );
    _delay_ms(500);
    led_set_all_to( grn );
    _delay_ms(500);
    }
    __builtin_unreachable();
}
