#include "sled1735.h"
#include <util/delay.h>
#include "main.h"
#include "keyscanner.h"
#include "wire-protocol.h"
#include <stdint.h>
#include "adc.h"

#define DETECT_ADC

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
        if(read_adc() < 500)
            led_set_all_to( red );
        else
            led_set_all_to( grn );
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
