#include <stdint.h>
#include <string.h>
#include <util/delay.h>
#include "main.h"
#include "led-spiout.h"
#include "wire-protocol.h"


/* SPI LED driver to send data to APA102 LEDs
 *
 * Preformatted data is sent to the micro and then
 * passed in via led_update_buffer(). The device
 * continuously outputs SPI data, refilling the SPI
 * output buffer from the SPI transfer complete interrupt.
 *
 * Data is double buffered (see notes below), however an update can
 * occur during any byte in the chain (we just guarantee it won't
 * happen mid-LED). The LED refresh rate is high enough that this
 * shouldn't matter.
 */


typedef union {
    uint8_t each[NUM_LEDS][LED_DATA_SIZE];
    uint8_t whole[LED_BUFSZ];
    uint8_t bank[NUM_LED_BANKS][LED_BANK_SIZE];
} led_buffer_t;

static volatile led_buffer_t led_buffer;

static volatile enum {
    START_FRAME,
    DATA,
    END_FRAME
} led_phase;

static volatile uint8_t global_brightness = 0xFF;

static volatile uint8_t index; /* next byte to transmit */
static volatile uint8_t subpixel = 0;

/* Update the transmit buffer with LED_BUFSZ bytes of new data */
void led_update_bank(uint8_t *buf, const uint8_t bank) {
    /* Double-buffering here is wasteful, but there isn't enough RAM on
       ATTiny48 to single buffer 32 LEDs and have everything else work
       unmodified. However there's enough RAM on ATTiny88 to double
       buffer 32 LEDs! And double buffering is simpler, less likely to
       flicker. */

    DISABLE_INTERRUPTS({
        memcpy((uint8_t *)led_buffer.bank[bank], buf, LED_BANK_SIZE);
    });
}

void led_set_one_to(uint8_t led, uint8_t *buf) {
    DISABLE_INTERRUPTS({
        memcpy((uint8_t *)led_buffer.each[led], buf, LED_DATA_SIZE);
    });

}

void led_set_global_brightness(uint8_t brightness) {
	if (brightness > 31) {
		return;
	}
	global_brightness = 0xE0 + brightness;
}

void led_set_all_to( uint8_t *buf) {
    DISABLE_INTERRUPTS({
        for(int8_t led=31; led>=0; led--) {
            memcpy((uint8_t *)led_buffer.each[led], buf, LED_DATA_SIZE);
        }
    });

}

void led_set_spi_frequency(uint8_t frequency) {
    /* Enable SPI master, MSB first
     * fOSC/16 speed (512KHz), the default
      Measured at about 300 Hz of LED updates
     */
    switch(frequency) {
    case LED_SPI_OFF:
        SPCR = 0x00;
        break;
    // fosc/2
    case LED_SPI_FREQUENCY_4MHZ:
        SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPIE);
        SPSR |= _BV(SPI2X);
        break;
    // fosc/4
    case LED_SPI_FREQUENCY_2MHZ:
        SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPIE);
        SPSR ^= _BV(SPI2X);
        break;
    // fosc/8
    case LED_SPI_FREQUENCY_1MHZ:
        SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPIE) | _BV(SPR0);
        SPSR |= _BV(SPI2X);
        break;
    // fosc/32
    case LED_SPI_FREQUENCY_256KHZ:
        SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPIE) | _BV(SPR1);
        SPSR |= _BV(SPI2X);
        break;
    // fosc/64
    case LED_SPI_FREQUENCY_128KHZ:
        SPCR = _BV(SPE)| _BV(MSTR) | _BV(SPIE) | _BV(SPR0) | _BV(SPR1);
        SPSR |= _BV(SPI2X);
        break;
    // fosc/128
    case LED_SPI_FREQUENCY_64KHZ:
        SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPIE) | _BV(SPR0) | _BV(SPR1);
        SPSR ^= _BV(SPI2X);
        break;
    // fosc/16
    case LED_SPI_FREQUENCY_512KHZ:
    default:
        SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPIE) | _BV(SPR0);
        SPSR ^= _BV(SPI2X);
        break;
    }
}


void led_init() {

    // Make sure all our LEDs start off dark
    uint8_t off[] = { 0x00,0x00,0x00};
    led_set_all_to(&off[0]);

    /* Set MOSI, SCK, SS all to outputs */
    DDRB = _BV(5)|_BV(3)|_BV(2);
    PORTB &= ~(_BV(5)|_BV(3)|_BV(2));

    led_set_spi_frequency(led_spi_frequency);

    /* Start transmitting the first byte of the start frame */
    led_phase = START_FRAME;
    SPDR = 0x0;
    index = 1;
    subpixel = 0;
}

/* Each time a byte finishes transmitting, queue the next one */
ISR(SPI_STC_vect) {
    switch(led_phase) {
    case START_FRAME:
        SPDR = 0;
        if(++index == 8) {
            led_phase = DATA;
            index = 0;
        }
        break;
    case DATA:
        if (++subpixel == 1) {
            SPDR = global_brightness;
        } else {
            SPDR = led_buffer.whole[index++];
            if(subpixel == 4) {
                subpixel = 0;
            }
        }

        if (index == LED_BUFSZ) {
            led_phase = END_FRAME;
            index = 0;
            subpixel = 0;
        }
        break;

    case END_FRAME:
	// The pwm reset frame needs to be 32 bits of 0 for SK9822 based LEDS
	// After that, we need at num_leds/2 more bits of 0 
	// For up to 64 LEDs, that means 64 bits of 0
        SPDR = 0x00;
        if(++index == 8) { /* NB: increase this number if ever >64 LEDs */
            led_phase = START_FRAME;
            index = 0;
        }
        break;
    }
}
