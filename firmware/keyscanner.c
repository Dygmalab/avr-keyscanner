#include <avr/interrupt.h>
#include <util/delay.h>
#include "debounce.h"
#include "wire-protocol.h"
#include "main.h"
#include "ringbuf.h"
#include "keyscanner.h"

debounce_t db[OUTPUT_COUNT];

// do_scan gets set any time we should actually do a scan
volatile uint8_t do_scan = 1;

void keyscanner_init(void) {

    CONFIGURE_OUTPUT_PINS;

    CONFIGURE_INPUT_PINS;

    debouncer_init(db, OUTPUT_COUNT);
    keyscanner_timer1_init();
}


void keyscanner_main(void) {
    uint8_t debounced_changes = 0;
    uint8_t pin_data;

    if (__builtin_expect(do_scan == 0, 1)) {
        return;
    }

    do_scan = 0;

    // For each enabled row...
    for (uint8_t output_pin = 0; output_pin < OUTPUT_COUNT; ++output_pin) {
	 // Toggle the output we want to check
         HIGH(PORT_OUTPUT, output_pin);

        /* We need a no-op for synchronization. 
	 * So says the datasheet in Section 10.2.5 */
        asm("nop");

        // Read pin data
        pin_data = PIN_INPUT;

        // Toggle the output we want to read back off
        LOW(PORT_OUTPUT,output_pin);

	// We don't have pull-down pins. Because of this, current can pretty easily leak across 
	// an entire column after a scan.
	// To pull the pins down, we flip them to outputs. By default, an output pin is driven low
	// so we don't need to explicitly drive it low.
	PINS_HIGH(DDR_INPUT, INPUT_PINMASK);

        // Debounce key state
        debounced_changes += debounce((pin_data) , db + output_pin);

	// The rows are inputs, set them back to input mode so we can read them 
	// on the next go round. By default, pullups are off, which is good because we want them off.
    	PINS_LOW(DDR_INPUT, INPUT_PINMASK);
    }

    // Most of the time there will be no new key events
    if (__builtin_expect(debounced_changes != 0, 0)) {
    	keyscanner_record_state();
    }
}

inline void keyscanner_record_state (void) {
    // The wire protocol expects data to be four rows of data, rather than 8 cols
    // of data. So we rotate it to match the original outputs
     uint8_t scan_data_as_rows[OUTPUT_COUNT]={0,0,0,0,0,0,0,0};
     for(int i=0; i<OUTPUT_COUNT; ++i){
    	for(int j=0; j<OUTPUT_COUNT; ++j){
      		scan_data_as_rows[i] = (  ( (db[j].state & (1 << (7-i) ) ) >> (7-i) ) << j ) | scan_data_as_rows[i];
    	}
    }

    // Snapshot the keystate to add to the ring buffer
    // Run this with interrupts off to make sure that
    // when we read from the ringbuffer, we always get 
    // four bytes representing a single keyboard state.
    DISABLE_INTERRUPTS({
            ringbuf_append( scan_data_as_rows[7] );
            ringbuf_append( scan_data_as_rows[6] );
            ringbuf_append( scan_data_as_rows[5] );
            ringbuf_append( scan_data_as_rows[4] );
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
