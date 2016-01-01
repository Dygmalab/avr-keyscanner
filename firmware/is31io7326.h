#pragma once

#include <stdint.h>

#define TWI_BASE_ADDRESS     0x58

typedef union {
    struct {
        uint8_t pp:3,
                od:3,
                keyState:1,
                dataNumber:1;
    };
    uint8_t val;
} key_t;


#define TWI_CMD_NONE 0x00
#define TWI_CMD_CFG  0x08
#define TWI_CMD_MASK_LED_BANK_0 0x20 // 0b00100000
#define TWI_CMD_MASK_LED_BANK_1 0x40 // 0b01000000
#define TWI_CMD_MASK_LED_BANK_2 0x60 // 0b01100000
#define TWI_CMD_MASK_LED_BANK_3 0x80 // 0b10000000

#define TWI_CMD_LED_BANK_FIXUP 0xE0  // 0b11100000


// Configuration register
extern uint8_t issi_config;


/* ACI: Auto Clear INT
 * 00: Auto clear INT disabled
 * 01: Auto clear INT in 5ms
 * 10: Auto clear INT in 10ms
 */
#define ISSI_CONFIG_ACI(CFG) (((CFG) >> 5) & 0x03)

/* DE: Input Port Filter Enable
 * 0: Input port filter disable
 * 1: Input port filter enable
 */
#define ISSI_CONFIG_DE(CFG)  (((CFG) >> 4) & 0x01)

/* SD: Key Scan Debounce Time
 * 0: Double debounce time (6ms, 8ms), 14ms total
 * 1: Normal debounce time (3ms, 4ms), 7ms total
 */
#define ISSI_CONFIG_SD(CFG)  (((CFG) >> 3) & 0x01)

/* LE: Long-pressed Key Detect Enable
 * 0: Disable
 * 1: Enable
 */
#define ISSI_CONFIG_LE(CFG)  (((CFG) >> 2) & 0x01)

/* LT: Long-pressed Key Detect Delay Time:
 * 00: 20ms
 * 01: 40ms
 * 10: 1s
 * 11: 2s
 */
#define ISSI_CONFIG_LT(CFG)  ((CFG) & 0x03)

// I²C driver functions
void issi_twi_data_received( uint8_t *buf, uint8_t bufsiz);
void issi_twi_data_requested( uint8_t *buf, uint8_t *bufsiz);

void issi_init(void);
