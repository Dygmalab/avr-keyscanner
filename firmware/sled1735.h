#pragma once
#include <stdint.h>
void sled_test();
void setup_spi();

uint8_t led_open_status[32];
uint8_t led_short_status[32];
uint8_t sled1735_status;

/* Call to begin transmitting LED data */

//void led_init(void);
//void led_update(void);

/* Call to change the speed at which we update the LEDs */
//void led_set_spi_frequency(uint8_t frequency);

/* Call this when you have new preformatted data for the LEDs */
void led_update_bank(uint8_t *buf, const uint8_t bank);

/* Call this when you want to set only a single LED */
void led_set_one_to(uint8_t led, uint8_t * led_data);

/* Call this when you want to set every LED to the same value */
void led_set_all_to(uint8_t * led_data);

/* Call this with a value between 0 and 31 to set the LED's global brightness */
//void led_set_global_brightness(uint8_t global_brightness);

void read_led_open_reg();
