#include "sled1735.h"
#include <util/delay.h>
#include "main.h"
#include "keyscanner.h"
#include "wire-protocol.h"
#include <stdint.h>

#define TEST_P 3

static inline void setup(void) {
    setup_spi(); // setup sled 1735 driver chip
    keyscanner_init();

    twi_init();

    DDRA |= (1<<1);
    //LOW(PORTA,TEST_P);
}
#define R 0
#define G 50
#define B 200
int main(void) {
    setup();
    /*
    uint8_t led[3] = { 100, 0, 0 };
//    led_set_one_to(0, led);

*/
/*
    uint8_t bank[24];
    for(int b = 0; b < 8; b ++ )
    {
        for(int i = 0; i < 24; i ++ )
            bank[i] = b * 20;
        led_update_bank(bank, b);
    }
    */
/*
    uint8_t buf[4] = { 0x03, 0x50, 0x00, 0x50 };
    uint8_t red[3] = { 255, 0, 0 };
    uint8_t blue[3] = { 0, 0, 255 };
    */
    uint8_t off[3] = { 0, 0, 0 };

    led_set_all_to( off );
    while(1) {
        keyscanner_main();
//        sled_test();
/*
    led_set_all_to( red );
    _delay_ms(500);
    led_set_all_to( blue );
    _delay_ms(500);
    */
    }
    __builtin_unreachable();
}
