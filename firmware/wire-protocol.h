#pragma once

#include <stdint.h>
#include "wire-protocol-constants.h"
#define TWI_BASE_ADDRESS     0x58

typedef union {
    struct {
        uint8_t row:2,
                col:3,
                keyState:1,
                keyEventsWaiting:1,
                eventReported:1;
    };
    uint8_t val;
} state_t;

#define DEVICE_VERSION 3




extern uint8_t led_spi_frequency;

// Default about 0.47ms between reads.
// This lets us do two scans per ms, which -might- let us send updates every ms.
#define KEYSCAN_INTERVAL_DEFAULT 14

// I²C driver functions
void twi_data_received( uint8_t *buf, uint8_t bufsiz);
void twi_data_requested( uint8_t *buf, uint8_t *bufsiz);

void twi_init(void);
