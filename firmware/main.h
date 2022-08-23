#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

// uint16_t joint;
// uint8_t ansi_iso;

#define ANSI 1
#define ISO 0

#if defined(__AVR_ATtiny88__) || defined(__AVR_attiny88__) || defined(__AVR_ATtiny48__) || defined(__AVR_attiny48__)
#include "config/attiny88.h"
#else
#error No port configuration found for hardware
#endif
#include "config/validate.h"

#define SET_OUTPUT(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define SET_INPUT(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))

#define HIGH(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define LOW(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))

#define DISABLE_INTERRUPTS(code) \
    do                           \
    {                            \
        cli();                   \
        {                        \
            code} sei();         \
    } while (0)