#include "is31io7326.h"
#include "main.h"
#include "ringbuf.h"
#include "twi-slave.h"

uint8_t issi_config = 0x10;
uint8_t pins_enabled = 0xff;

void issi_init(void)
{

    TWI_Rx_Data_Callback = issi_twi_data_received;
    TWI_Tx_Data_Callback = issi_twi_data_requested;

    // TODO: set TWI_Tx_Data_Callback and TWI_Rx_Data_Callback
    TWI_Slave_Initialise(TWI_BASE_ADDRESS | AD01());
    sei();
}

static uint8_t issi_twi_command = TWI_CMD_NONE;

void issi_twi_data_received(uint8_t *buf, uint8_t bufsiz) {
    if (__builtin_expect(bufsiz != 0, 1)) {
        if (buf[0] == TWI_CMD_CFG) {
            if (bufsiz > 1) {
	            // SET configuration
	            issi_config = buf[1];
	        } else {
	            // GET configuration
	            issi_twi_command = TWI_CMD_CFG;
	        }
        } else if (buf[0] == TWI_CMD_PINS) {
            if (bufsiz > 1) {
                // SET the pins we should look at
                pins_enabled = buf[1];
            } else {
                // GET the pins we should look at
                issi_twi_command = TWI_CMD_PINS;
            }

        }
    }
}

void issi_twi_data_requested(uint8_t *buf, uint8_t *bufsiz) {
    if (__builtin_expect(*bufsiz != 0, 1)) {
        if (issi_twi_command == TWI_CMD_NONE) {
            // Key Status Register
            if (ringbuf_empty()) {
                *bufsiz = 0;
            } else {
                key_t key;
                key.val = ringbuf_pop();
                if (ringbuf_empty()) {
                    key.dataNumber = 0;
                } else {
                    key.dataNumber = 1;
                }
                buf[0] = key.val;
                *bufsiz = 1;
            }
        }
        else if (issi_twi_command == TWI_CMD_PINS) {
            // Configuration Register
            buf[0] = pins_enabled;
            *bufsiz = 1;
            // reset the twi command on the wire
            issi_twi_command = TWI_CMD_NONE;
        }  
        else if (issi_twi_command == TWI_CMD_CFG) {
            // Configuration Register
            buf[0] = issi_config;
            *bufsiz = 1;
            // reset the twi command on the wire
            issi_twi_command = TWI_CMD_NONE;
        }  
    }
}
