#include "sled1735.h"
#include "main.h"

#define TEST_P 3

static inline void setup(void) {
    setup_spi(); // setup sled 1735 driver chip
    twi_init();

    //DDRA = (1<<TEST_P);
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

    for(int i = 0; i < 24; i ++ )
        bank[i] = 10;
    led_update_bank(bank, 0);
    uint8_t bank[24];
    for(int i = 0; i < 24; i ++ )
        bank[i] = 150;
    led_update_bank(bank, 1);
*/
//    uint8_t buf[4] = { 0x03, 0x00, 0x00, 0x50 };
 //   led_set_all_to( &buf[1]);



    while(1) {
//        sled_test();
    }
    __builtin_unreachable();
}
