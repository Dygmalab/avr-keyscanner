#pragma once

#include <stdint.h>

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

#define DEVICE_VERSION 2

#define TWI_CMD_NONE 0x00
#define TWI_CMD_CFG 0x01
#define TWI_CMD_LED_DISABLE 0x02
#define TWI_CMD_VERSION 0x03
#define TWI_CMD_DEBOUNCE_DELAY 0x04
#define TWI_CMD_LED_SET_ALL_TO 0x05
#define TWI_CMD_LED_SET_ONE_TO 0x06
#define TWI_CMD_COLS_USE_PULLUPS 0x07




#define TWI_CMD_LED_BASE 0x80


#define TWI_REPLY_NONE 0x00
#define TWI_REPLY_KEYDATA 0x01

// Configuration register
extern uint8_t device_config;

// I²C driver functions
void twi_data_received( uint8_t *buf, uint8_t bufsiz);
void twi_data_requested( uint8_t *buf, uint8_t *bufsiz);

void twi_init(void);
