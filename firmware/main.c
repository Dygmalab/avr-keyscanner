#include "sled1735.h"
#include "main.h"
#include "keyscanner.h"
#include "wire-protocol.h"
#include <stdint.h>
#include <avr/wdt.h>

static inline void setup(void)
{
    setup_spi(); // setup sled 1735 driver chip
    keyscanner_init();

    // ansi iso reading - up to v4.9 pcbs use ANSI pullup, ISO floating
    // but chip doesn't have configurable pulldowns, only pullups
    // set pin to be low output first, to drain any voltage on pin's capacitance
    SET_OUTPUT(DDRB, 1);
    LOW(PORTB, 1);

    // then set it input
    SET_INPUT(DDRB, 1);
    // leave enough time full the pullup to change the voltage on the pin
    // asm("nop");

    // then read: 0 = ISO, 2 = ANSI
    // ansi_iso = (PINB & _BV(1)) == 2 ? ANSI : ISO;

    // setup watchdog
    wdt_enable(WDTO_250MS);
    // initialise twi
    twi_init();
}

int main(void)
{
    setup();

    while (1)
    {
        if (keyscanner_main())
            wdt_reset();
    }
    __builtin_unreachable();
}
