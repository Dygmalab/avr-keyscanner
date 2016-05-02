#include "wire-protocol.h"
#include "keyscanner.h"
#include "led-spiout.h"

static inline void setup(void)
{
    keyscanner_init();
    issi_init();
}

int main(void)
{
    setup();
    led_init();
    while(1){
        keyscanner_main();
    }
    __builtin_unreachable();
}
