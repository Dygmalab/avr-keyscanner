#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

#if ! defined (PRODUCT_ID)
#error No hardware definition included from Makefile
#endif

#include "config/validate.h"

#define SET_OUTPUT(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define SET_INPUT(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))

#define HIGH(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define LOW(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))

#define PINS_HIGH(sfr, bitmask) ( _SFR_BYTE(sfr) |= bitmask)
#define PINS_LOW(sfr, bitmask) ( _SFR_BYTE(sfr) &= ~bitmask)

#define EXPECT_FALSE 0
#define EXPECT_TRUE 1


#define DISABLE_INTERRUPTS(code) do{ \
    cli(); \
    { \
        code \
    } \
    sei(); \
}while(0)
