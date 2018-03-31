#include <avr/interrupt.h>
#include <util/delay.h>
#include "debounce.h"
#include "wire-protocol.h"
#include "main.h"
#include "ringbuf.h"
#include "keyscanner.h"

debounce_t db[] = {
    {0x00, 0x00, 0xFF},
    {0x00, 0x00, 0xFF},
    {0x00, 0x00, 0xFF},
    {0x00, 0x00, 0xFF},
    {0x00, 0x00, 0xFF},
    {0x00, 0x00, 0xFF},
    {0x00, 0x00, 0xFF},
    {0x00, 0x00, 0xFF}
};

// do_scan gets set any time we should actually do a scan
volatile uint8_t do_scan = 1;

void keyscanner_init(void) {

    // Write to rows - we only use some of the pins in the row port
    DDR_ROWS &= ~ROW_PINMASK;
    PORT_ROWS &= ~ROW_PINMASK;

    // Read from cols -- We use all 8 bits of cols
    DDR_COLS  = 0xFF;
    // Turn on the Pullups
    PORT_COLS = 0x00;

    // Assert comm_en so we can use the interhand transcievers
    // (Until comm_en on the i2c transcievers is pulled high,
    //  they're disabled)

    // PC7 is on the same port as the four row pins.
    // We refer to it here as PORTC because
    // we're not using it as part of the keyscanner
    HIGH(PORTC,7);
    SET_OUTPUT(DDRC,7);

    keyscanner_timer1_init();
}


void keyscanner_main(void) {
    uint8_t debounced_changes = 0;
    uint8_t pin_data;

    if (__builtin_expect(do_scan == 0, 1)) {
        return;
    }

    // For each enabled row...
    for (uint8_t col = 0; col < 8; ++col) {
        // Reset all of our row pins, then
        // set the one we want to read as low
        HIGH(PORT_COLS, col);
        /* We need a no-op for synchronization. So says the datasheet
         * in Section 10.2.5 */
        asm("nop");
        pin_data = PIN_ROWS;

	// Drive the rows low to try to clear them;
	DDR_ROWS |= ROW_PINMASK;
	PORT_ROWS &= ~ROW_PINMASK;
        LOW(PORT_COLS,col);
        // Debounce key state
        debounced_changes += debounce((pin_data) , db + col);
    DDR_ROWS &= ~ROW_PINMASK;
    PORT_ROWS &= ~ROW_PINMASK;
    }

    // Most of the time there will be no new key events
    if (__builtin_expect(debounced_changes == 0, 1)) {
        // Only run the debouncing delay when we haven't successfully found
        // a debounced event

        // XXX TODO make sure this isn't crazy. could this 
        // cause us to do reads too fast and mis-debounce
        // some secondary key while we successfully debounce a
        // first key.
        do_scan = 0;
        return;
    }

    // Snapshot the keystate to add to the ring buffer
    // Run this with interrupts off to make sure that
    // when we read from the ringbuffer, we always get 
    // four bytes representing a single keyboard state.
    //
    //
     uint8_t rows2[8]={0,0,0,0,0,0,0,0};
  for(int i=0; i<8; ++i){
    for(int j=0; j<8; ++j){
      rows2[i] = (  ( (db[j].state & (1 << (7-i) ) ) >> (7-i) ) << j ) | rows2[i];

    }
  }



    DISABLE_INTERRUPTS({
            ringbuf_append( rows2[7] );
            ringbuf_append( rows2[6] );
            ringbuf_append( rows2[5] );
            ringbuf_append( rows2[4] );
    });
}

// initialize timer, interrupt and variable
void keyscanner_timer1_init(void) {

    // set up timer with prescaler = 256 and CTC mode
    TCCR1B |= _BV(WGM12)| _BV( CS12);

    // initialize counter
    TCNT1 = 0;

    // initialize compare value
    OCR1A = KEYSCAN_INTERVAL_DEFAULT;

    // enable compare interrupt
    TIMSK1 |= _BV(OCIE1A);

    // enable global interrupts
    sei();
}

// interrupt service routine (ISR) for timer 1 A compare match
ISR(TIMER1_COMPA_vect) {
    do_scan = 1; // Yes! Let's do a scan
}
