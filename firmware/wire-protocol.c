#include "wire-protocol.h"
#include <string.h>
#include "main.h"
#include "ringbuf.h"
#include "twi-slave.h"
#include "led-spiout.h"


uint8_t led_spi_frequency = LED_SPI_FREQUENCY_DEFAULT;

void twi_init(void) {

    TWI_Rx_Data_Callback = twi_data_received;
    TWI_Tx_Data_Callback = twi_data_requested;

    // TODO: set TWI_Tx_Data_Callback and TWI_Rx_Data_Callback
    DDRB |= _BV(0) | _BV(1);

    SET_INPUT(DDRB,0);
    SET_INPUT(DDRB,1);
    LOW(PORTB,0);
    LOW(PORTB,1);

    TWI_Slave_Initialise(TWI_BASE_ADDRESS | AD01());
    sei();
    DDRC |= _BV(1); // PC1 is pin 24
}

static uint8_t twi_command = TWI_CMD_NONE;

void twi_data_received(uint8_t *buf, uint8_t bufsiz) {
    // if the upper four bits of the byte say this is an LED cmd
    // this is the most common case. It's also the only case where
    // we can't just compare buf[0] to a static value
    if (__builtin_expect( ((buf[0] & 0xf0) == TWI_CMD_LED_BASE),1))  {
        led_update_bank(&buf[1], buf[0] & 0x0f); // the lowest four bits are the bank #
        return;
    }

    switch (buf[0]) {
    case TWI_CMD_KEYSCAN_INTERVAL:
        if (bufsiz == 2 ) {
            // SET the delay
            OCR1A = buf[1];
        } else {
            // GET configuration
            twi_command = TWI_CMD_KEYSCAN_INTERVAL;
        }
        break;
/*
    case TWI_CMD_LED_SPI_FREQUENCY:
        if (bufsiz == 2 ) {
            led_spi_frequency = buf[1];
            led_set_spi_frequency(led_spi_frequency);
        } else {
            twi_command = TWI_CMD_LED_SPI_FREQUENCY;
        }
        break;
    */

    case TWI_CMD_LED_SET_ALL_TO:
        if (bufsiz == 4 ) {
            led_set_all_to(&buf[1]);
        }
        break;

    case TWI_CMD_LED_SET_ONE_TO:
        if (bufsiz == 5 ) {
            led_set_one_to(buf[1],&buf[2]);
        }
        break;

    case TWI_CMD_VERSION:
        twi_command = TWI_CMD_VERSION;
        break;
        /*
    case TWI_CMD_LED_GLOBAL_BRIGHTNESS:
	led_set_global_brightness(buf[1]);
	break;
    */
    }
}

uint8_t key_substate;

void twi_data_requested(uint8_t *buf, uint8_t *bufsiz) {
    if (__builtin_expect(*bufsiz != 0, 1)) {
        switch (twi_command) {
        case TWI_CMD_NONE:
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
                buf[1] = ringbuf_pop();
                buf[2] = ringbuf_pop();
                buf[3] = ringbuf_pop();
                buf[4] = ringbuf_pop();
                buf[5] = ringbuf_pop();
                *bufsiz=6;
            }
            break;
        case TWI_CMD_VERSION:
            buf[0] = DEVICE_VERSION;
            *bufsiz = 1;
            twi_command = TWI_CMD_NONE;
            break;
        case TWI_CMD_KEYSCAN_INTERVAL:
            buf[0] = OCR1A;
            *bufsiz = 1;
            twi_command = TWI_CMD_NONE;
            break;
        case TWI_CMD_LED_SPI_FREQUENCY:
            buf[0] = led_spi_frequency;
            *bufsiz = 1;
            twi_command = TWI_CMD_NONE;
            break;
        default:
            buf[0] = 0x01;
            *bufsiz = 1;
            break;
        }
    }
}
