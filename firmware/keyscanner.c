#include <avr/interrupt.h>
#include <util/delay.h>
#include "main.h"
#include DEBOUNCER
#include "wire-protocol.h"
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

    if (__builtin_expect(do_scan == 0, EXPECT_TRUE)) {
        return;
    }

    do_scan = 0;

    // For each enabled row...
    for (uint8_t output_pin = 0; output_pin < OUTPUT_COUNT; ++output_pin) {

	REINIT_INPUT_PINS;

	 // Toggle the output we want to check
         ACTIVATE_OUTPUT_PIN(output_pin);

        /* We need a no-op for synchronization. 
	 * So says the datasheet in Section 10.2.5 */
        asm("nop");

        // Read pin data
        pin_data = PIN_INPUT;

        // Toggle the output we want to read back off
        DEACTIVATE_OUTPUT_PIN(output_pin);

	CLEANUP_INPUT_PINS;

        // Debounce key state
        debounced_changes += debounce((pin_data) , db + output_pin);

    }

    // Most of the time there will be no new key events
    if (__builtin_expect(debounced_changes != 0, EXPECT_FALSE)) {
    	RECORD_KEY_STATE;
    }
}

inline void keyscanner_record_state_rotate_ccw (void) {
    // The wire protocol expects data to be four rows of data, rather than 8 cols
    // of data. So we rotate it to match the original outputs
     uint8_t scan_data_as_rows[OUTPUT_COUNT]={0,0,0,0,0,0,0,0};
     for(int i=0; i<OUTPUT_COUNT; ++i){
    	for(int j=0; j<OUTPUT_COUNT; ++j){
      		scan_data_as_rows[i] = (  ( (db[j].state & (1 << (7-i) ) ) >> (7-i) ) << j ) | scan_data_as_rows[i];
    	}
    }
    keyscanner_ringbuf_update( scan_data_as_rows[7], scan_data_as_rows[6], scan_data_as_rows[5], scan_data_as_rows[4]);
}

inline void keyscanner_record_state (void) {
    keyscanner_ringbuf_update( db[0].state, db[1].state, db[2].state, db[3].state );
}

inline void keyscanner_ringbuf_update(uint8_t row1, uint8_t row2, uint8_t row3, uint8_t row4) {

    // Snapshot the keystate to add to the ring buffer
    // Run this with interrupts off to make sure that
    // when we read from the ringbuffer, we always get 
    // four bytes representing a single keyboard state.
    DISABLE_INTERRUPTS({
            ringbuf_append(row1);
            ringbuf_append(row2);
            ringbuf_append(row3);
            ringbuf_append(row4);
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
