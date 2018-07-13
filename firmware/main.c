#include <util/delay.h>
#include "main.h"
#include "wire-protocol.h"
#include <stdint.h>

static inline void setup(void) {
    twi_init();

}

void sig_pin(bool set)
{
    if(set)
    {
    // pull down to 0
    SET_OUTPUT(DDRB,0);
    LOW(PORTB,0);

    }
    else
    {
    // let float
    SET_INPUT(DDRB,0);
    LOW(PORTB,0);
    }
}

int main(void) {
    setup();

    while(1) {
    /*
    sig_pin(true);
    _delay_ms(1000);
    sig_pin(false);
    _delay_ms(1000);
    */
    }
    __builtin_unreachable();
}
