#include "wire-protocol.h"
#include <string.h>
#include "main.h"
#include "ringbuf.h"
#include "twi-slave.h"
#include "led-spiout.h"
uint8_t issi_config = 0x10;

void issi_init(void) {

    TWI_Rx_Data_Callback = issi_twi_data_received;
    TWI_Tx_Data_Callback = issi_twi_data_requested;

    // TODO: set TWI_Tx_Data_Callback and TWI_Rx_Data_Callback
    TWI_Slave_Initialise(TWI_BASE_ADDRESS | AD01());
    sei();
}

static uint8_t issi_twi_command = TWI_CMD_NONE;

void issi_twi_data_received(uint8_t *buf, uint8_t bufsiz) {
    if (__builtin_expect(bufsiz <=2, 0)) {
        if (buf[0] == TWI_CMD_CFG) {
            if (bufsiz > 1) {
                // SET configuration
                issi_config = buf[1];
            } else {
                // GET configuration
                issi_twi_command = TWI_CMD_CFG;
            }
        } else if (buf[0] == TWI_CMD_LED_DISABLE) {
            led_disable();
        }
    }
    // if the upper four bits of the byte say this is an LED cmd
    else if ((buf[0] & 0xf0) == TWI_CMD_LED_BASE)  {
        led_update_bank(&buf[1], buf[0] & 0x0f); // the lowest four bits are the bank #
    }
}

uint8_t key_substate;

void issi_twi_data_requested(uint8_t *buf, uint8_t *bufsiz) {
    if (__builtin_expect(*bufsiz != 0, 1)) {
        if (issi_twi_command == TWI_CMD_NONE) {
            // Keyscanner Status Register
            if (ringbuf_empty()) {
                // Nothing in the ring buffer is the same thing as all keys released
                // Really, we _should_ be able to return a single byte here, but
                // Jesse is too clueless to figure out how to get I2C to signal
                // a 'short' response
                buf[0]=TWI_REPLY_NONE;
                *bufsiz=1;
            } else {
                buf[0]=TWI_REPLY_KEYDATA;
                for (uint8_t i = 1; i<5; i++ ) {

                    buf[i] = ringbuf_pop();
                }
                *bufsiz=5;
            }
        } else if (issi_twi_command == TWI_CMD_CFG) {
            // Configuration Register
            buf[0] = issi_config;
            *bufsiz = 1;
            // reset the twi command on the wire
            issi_twi_command = TWI_CMD_NONE;
        } else {
            buf[0] = 0x01;
            *bufsiz = 1;
        }
    }
}
