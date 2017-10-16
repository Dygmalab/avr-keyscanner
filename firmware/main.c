#include "sled1735.h"
#include "main.h"

#define TEST_P 3

static inline void setup(void) {
    setup_spi(); // setup sled 1735 driver chip

    //DDRA = (1<<TEST_P);
    //LOW(PORTA,TEST_P);
}

int main(void) {
    setup();

    while(1) {
        sled_test();
        sled_test();
    }
    __builtin_unreachable();
}
