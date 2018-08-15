#include "sled1735.h"
#include <util/delay.h>
#include "main.h"
#include "keyscanner.h"
#include "wire-protocol.h"
#include <stdint.h>
#include "adc.h"

//#define DETECT_ADC

uint8_t red[3] = { 255, 0, 0 };
uint8_t grn[3] = { 0, 255, 0 };
uint8_t off[3] = { 0, 0, 0 };


static inline void setup(void) {
    setup_spi(); // setup sled 1735 driver chip
    keyscanner_init();

    #ifdef DETECT_ADC

    // if dpf is providing power to us
    while(read_adc(ADC_CC) > 40)
    {
        led_set_all_to(red); 
        _delay_ms(100);
    }
    led_set_all_to(off); 
    #endif
    twi_init();
}

int main(void) {
    setup();

    while(1) {
        keyscanner_main();
    }
    __builtin_unreachable();
}
