#include "sled1735.h"
#include "main.h"

#define TEST_P 3

static inline void setup(void) {
    setup_spi();

    DDRA = (1<<TEST_P);
}

int main(void) {
    setup();

    while(1) {
        HIGH(PORTA,TEST_P);
        sled_test();
        LOW(PORTA,TEST_P);
        sled_test();
    }
    __builtin_unreachable();
}
