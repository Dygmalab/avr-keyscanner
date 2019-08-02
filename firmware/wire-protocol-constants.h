#pragma once

#define TWI_CMD_NONE 0x00
#define TWI_CMD_VERSION 0x01
#define TWI_CMD_KEYSCAN_INTERVAL 0x02
#define TWI_CMD_LED_SET_ALL_TO 0x03
#define TWI_CMD_LED_SET_ONE_TO 0x04
#define TWI_CMD_COLS_USE_PULLUPS 0x05
#define TWI_CMD_LED_SPI_FREQUENCY 0x06
#define TWI_CMD_LED_GLOBAL_BRIGHTNESS 0x07

#define TWI_CMD_SLED_STATUS 0x08
#define TWI_CMD_LED_OPEN 0x09
#define TWI_CMD_LED_SHORT 0x0A
#define TWI_CMD_JOINED 0x0B
#define TWI_CMD_ANSI_ISO 0x0C
#define TWI_CMD_SLED_CURRENT 0x0D
#define TWI_CMD_SLED_SELF_TEST 0x0E
#define TWI_CMD_SLED_SYS_TEMP 0x0F

#define LED_SPI_FREQUENCY_4MHZ      0x07
#define LED_SPI_FREQUENCY_2MHZ      0x06
#define LED_SPI_FREQUENCY_1MHZ      0x05
#define LED_SPI_FREQUENCY_512KHZ    0x04
#define LED_SPI_FREQUENCY_256KHZ    0x03
#define LED_SPI_FREQUENCY_128KHZ    0x02
#define LED_SPI_FREQUENCY_64KHZ     0x01
#define LED_SPI_OFF                 0x00


// 512KHZ seems to be the sweet spot in early testing
// so make it the default
#define LED_SPI_FREQUENCY_DEFAULT LED_SPI_FREQUENCY_512KHZ


#define TWI_CMD_LED_BASE 0x80

#define TWI_REPLY_NONE 0x00
#define TWI_REPLY_KEYDATA 0x01
