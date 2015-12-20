/* APA102 LED driver, receives data as a precanned buffer then outputs it via SPI interrupts */
#pragma once

/* Number of LEDs in the chain.
   Max is ~8 (maybe less) on ATTiny48, 32 on ATTiny88. */
#define NUM_LEDS 32

#define LED_BUFSZ (4*NUM_LEDS)

/* Call to begin transmitting LED data */
void led_init(void);

/* Call this when you have new preformatted data for the LEDs */
void led_update_buffer(const uint8_t *buf);
