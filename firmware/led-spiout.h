/* APA102 LED driver, receives data as a precanned buffer then outputs it via SPI interrupts */
#pragma once

/* Number of LEDs in the chain.
   Max is ~8 (maybe less) on ATTiny48, 32 on ATTiny88. */
#define NUM_LEDS 32
#define NUM_LEDS_PER_BANK 8

#define LED_DATA_SIZE 4
#define LED_BUFSZ (LED_DATA_SIZE *NUM_LEDS)
#define LED_BANK_SIZE (LED_DATA_SIZE*NUM_LEDS_PER_BANK)

/* Call to begin transmitting LED data */
void led_init(void);

/* Call this when you have new preformatted data for the LEDs */
void led_update_bank(uint8_t *buf, const uint8_t bank);

