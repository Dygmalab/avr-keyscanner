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
    setup_adc();
    while(true)
    {
        // 360mv (70ADC) if dpf is providing power to us
        // think I have adc wrong, as it works up to about 300count.
        if(read_adc() > 150)
            led_set_one_to(56, red); 
        else
            led_set_one_to(56, grn);
            //led_set_all_to( grn );
        _delay_ms(100);
    }
    #endif
    twi_init();

    // joint detect setup
    SET_INPUT(DDRA,JOINT_PIN);
    HIGH(PORTA,JOINT_PIN);
}

int main(void) {
    setup();

    while(1) {
        keyscanner_main();
    }
    __builtin_unreachable();
}
