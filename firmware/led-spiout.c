#include <stdint.h>
#include <string.h>
#include <util/delay.h>
#include "main.h"
#include "led-spiout.h"
#include "wire-protocol.h"


/* SPI LED driver to send data to APA102 LEDs
 *
 * Preformatted data is sent to the micro and then
 * passed invia led_update_buffer(). The device
 * continuously outputs SPI data, refilling the SPI
 * output buffer from the SPI transfer complete interrupt.
 *
 * Data is double buffered (see notes below), however an update can
 * occur during any byte in the chain (we just guarantee it won't
 * happen mid-LED). The LED refresh rate is high enough that this
 * shouldn't matter.
 */


typedef union {
    uint8_t whole[LED_BUFSZ];
    uint8_t bank[NUM_LED_BANKS][LED_BANK_SIZE];
} led_buffer_t;

static volatile led_buffer_t led_buffer;

static volatile enum {
    START_FRAME,
    DATA,
    END_FRAME
} led_phase;

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


/* Turn off the LEDs.
 * TODO: A future implementation would stop sending updates to the LEDs
 *       after setting them to black
 */

void led_disable() {
    DISABLE_INTERRUPTS({
        memset(&led_buffer.whole, 0x00, sizeof(led_buffer.whole));
    });
}

void led_init() {

    // Make sure all our LEDs start off dark
    led_disable();

    /* Set MOSI, SCK, SS all to outputs */
    DDRB = _BV(5)|_BV(3)|_BV(2);
    PORTB &= ~(_BV(5)|_BV(3)|_BV(2));

    /* Enable SPI master, MSB first, fOSC/16 speed
     * (512KHz)

      Measured at about 300 Hz of LED updates

     */
    SPCR = _BV(SPE)|_BV(MSTR)|_BV(SPIE) | _BV(SPR0);

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
        if(index == 3 ) {
            led_phase = DATA;
            index = 0;
        } else {
            index++;
        }
        break;
    case DATA:
        if (subpixel++ ==  0) {
            SPDR = 0xff;
        } else {
            SPDR = led_buffer.whole[index++];
            if(subpixel == 4) {
                subpixel = 0;
            }
        }

        if(index == LED_BUFSZ) {
            led_phase = END_FRAME;
            index = 0;
            subpixel=0;
        }
        break;
    case END_FRAME:
        SPDR = 0x00;
        if(index == 3) { /* NB: increase this number if ever >64 LEDs */
            led_phase = START_FRAME;
            index = 0;
        } else {
            index++;
        }
        break;
    }
}
