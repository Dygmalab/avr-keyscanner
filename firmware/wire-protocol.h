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


#define TWI_CMD_NONE 0x00
#define TWI_CMD_CFG 0x01
#define TWI_CMD_LED_DISABLE 0x02
#define TWI_CMD_LED_BASE 0x80
#define TWI_CMD_VERSION 0x03


#define TWI_REPLY_NONE 0x00
#define TWI_REPLY_KEYDATA 0x01

// Configuration register
extern uint8_t device_config;


/* ACI: Auto Clear INT
 * 00: Auto clear INT disabled
 * 01: Auto clear INT in 5ms
 * 10: Auto clear INT in 10ms
 */
#define DEVICE_CONFIG_ACI(CFG) (((CFG) >> 5) & 0x03)

/* DE: Input Port Filter Enable
 * 0: Input port filter disable
 * 1: Input port filter enable
 */
#define DEVICE_CONFIG_DE(CFG)  (((CFG) >> 4) & 0x01)

/* SD: Key Scan Debounce Time
 * 0: Double debounce time (6ms, 8ms), 14ms total
 * 1: Normal debounce time (3ms, 4ms), 7ms total
 */
#define DEVICE_CONFIG_SD(CFG)  (((CFG) >> 3) & 0x01)

/* LE: Long-pressed Key Detect Enable
 * 0: Disable
 * 1: Enable
 */
#define DEVICE_CONFIG_LE(CFG)  (((CFG) >> 2) & 0x01)

/* LT: Long-pressed Key Detect Delay Time:
 * 00: 20ms
 * 01: 40ms
 * 10: 1s
 * 11: 2s
 */
#define DEVICE_CONFIG_LT(CFG)  ((CFG) & 0x03)

// I²C driver functions
void twi_data_received( uint8_t *buf, uint8_t bufsiz);
void twi_data_requested( uint8_t *buf, uint8_t *bufsiz);

void twi_init(void);
