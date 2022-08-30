#include "wire-protocol.h"
#include <string.h>
#include "main.h"
#include "twi-slave.h"
#include "sled1735.h"
#include <util/crc16.h>
#include "keyscanner.h"

void twi_init(void) {

    TWI_Rx_Data_Callback = twi_data_received;
    TWI_Tx_Data_Callback = twi_data_requested;

    // i2c address set by combining TWI_BASE_ADDRESS with the value on PORTB0 (pin 12)
    SET_INPUT(DDRB,0);
    LOW(PORTB,0);

    TWI_Slave_Initialise(TWI_BASE_ADDRESS | AD01());
    sei();
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

    if(bufsiz < 2) // must be more than 2 to have a cksum
        return;
    uint16_t crc16 = 0xffff;
    uint16_t rx_cksum = (buf[bufsiz - 2] << 8) + buf[bufsiz - 1];
    uint8_t *bufferPtr;
    bufferPtr = buf;

    for (uint8_t i = 0; i < bufsiz - 2; i++) {
//        crc16 = _crc16_update(crc16, *bufferPtr);
        crc16 = _crc_ccitt_update(crc16, *bufferPtr);
        bufferPtr++;
    }

    // check received CRC16
    if (crc16 != rx_cksum)
        return;


    bufsiz -= 2; // set bufsiz to what it was before cksum was added

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

    case TWI_CMD_SLED_CURRENT:
        twi_command = TWI_CMD_SLED_CURRENT;
        if(bufsiz == 2)
            set_current(buf[1]);
        break;

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

    case TWI_CMD_SLED_SYS_TEMP:
        twi_command = TWI_CMD_SLED_SYS_TEMP;
        break;

    case TWI_CMD_JOINED:
        twi_command = TWI_CMD_JOINED;
        break;

    case TWI_CMD_VERSION:
        twi_command = TWI_CMD_VERSION;
        break;

    case TWI_CMD_ANSI_ISO:
        twi_command = TWI_CMD_ANSI_ISO;
        break;

    case TWI_CMD_SLED_STATUS:
        twi_command = TWI_CMD_SLED_STATUS;
        break;

    case TWI_CMD_LED_OPEN:
        twi_command = TWI_CMD_LED_OPEN;
        break;

    case TWI_CMD_LED_SHORT:
        twi_command = TWI_CMD_LED_SHORT;
        break;

    case TWI_CMD_SLED_SELF_TEST:
        // how long does this take? with WDT might trigger a reset
        if(bufsiz == 2)
            self_test(buf[1]);
        break;
    }
}

uint8_t key_substate;

void twi_data_requested(uint8_t *buf, uint8_t *bufsiz) {
    if (__builtin_expect(*bufsiz != 0, 1)) {
        switch (twi_command) {
        case TWI_CMD_NONE:
            // Keyscanner Status Register
            if (!new_key_state) {
                // Nothing in the ring buffer is the same thing as all keys released
                buf[0]=TWI_REPLY_NONE;
                buf[1] = 0;
                buf[2] = 0;
                buf[3] = 0;
                buf[4] = 0;
                buf[5] = 0;
                *bufsiz=6;
            } else {
                buf[0]=TWI_REPLY_KEYDATA;
                buf[1] = key_state[0];
                buf[2] = key_state[1];
                buf[3] = key_state[2];
                buf[4] = key_state[3];
                buf[5] = key_state[4];
                new_key_state = false;

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
            buf[0] = 0x05;
            *bufsiz = 1;
            twi_command = TWI_CMD_NONE;
            break;
        case TWI_CMD_SLED_STATUS:
            buf[0] = 0xFF;
            *bufsiz = 1;
            twi_command = TWI_CMD_NONE;
            break;
        case TWI_CMD_SLED_CURRENT:
            buf[0] = 63;
            *bufsiz = 1;
            twi_command = TWI_CMD_NONE;
            break;
        case TWI_CMD_ANSI_ISO:
            buf[0] = ansi_iso;
            *bufsiz = 1;
            twi_command = TWI_CMD_NONE;
            break;
        case TWI_CMD_SLED_SYS_TEMP:
            buf[0] = 63;
            *bufsiz = 1;
            twi_command = TWI_CMD_NONE;
            break;
        case TWI_CMD_JOINED:
        {
            // send low byte
            buf[0] = joint & 0x00FF;
            // then high byte
            buf[1] = joint >> 8;
            *bufsiz = 2;
            twi_command = TWI_CMD_NONE;
            break;
        }
        default:
            buf[0] = 0x01;
            *bufsiz = 1;
            break;
        }
        
        // calc cksum
        uint16_t crc16 = 0xffff;
        uint8_t *bufferPtr;
        bufferPtr = buf;
        for (uint8_t i = 0; i < *bufsiz; i++) {
            //crc16 = _crc16_update(crc16, *bufferPtr);
            crc16 = _crc_ccitt_update(crc16, *bufferPtr);
            bufferPtr++;
        }
        
        // append cksum high byte and low byte
        buf[*bufsiz] = crc16 >> 8;
        buf[*bufsiz + 1] = crc16;
        *bufsiz += 2; 
    }
}
