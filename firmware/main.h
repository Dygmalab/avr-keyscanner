#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

#if defined (__AVR_ATtiny48__) || defined (__AVR_ATtiny88__)
# include "config/attiny88.h"
#else
# error No port configuration found for hardware
#endif
#include "config/validate.h"

#define SET_OUTPUT(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define SET_INPUT(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))

#define HIGH(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define LOW(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))

#define DISABLE_INTERRUPTS(code) do{ \
    cli(); \
    { \
        code \
    } \
    sei(); \
}while(0)
