#pragma once

/*
 * Application considerations:
 * The ATtiny48/88 has three 8-bit ports available.
 * Two have special functionality of interest to the application:
 *     PORTB: used by ISP
 *     PORTC: used by TWI/I²C
 *
 */


#define PRODUCT_ID keyboardio-model-01


// Debouncer config

#define DEBOUNCER "debounce-integrator.h"
//#define DEBOUNCER "debounce-counter.h"
//#define DEBOUNCER "debounce-none.h"
//#define DEBOUNCER "debounce-split-counters-and-lockouts.h"
//#define DEBOUNCER "debounce-split-counters.h"


// Should we be treating our columns as outputs or our rows?
// If the columns are outputs, our keyscanning loop has to do twice as much work,
// as we have 8 columns and four rows. However, it means that we're able to drive
// the column pins high, keeping the row pins low. When doing a read, we're pushing
// a lot more power through the circuit. If there's a marginal connection, this should,
// theoretically, get us much cleaner reads.
//
// If COLS_ARE_OUTPUTS is NOT defined, we treat our rows as outputs.
// To do this we, turn on pull-up resistors on the columns and drive the rows
// LOW when we want to read them. This is a pretty traditional scheme. It's less work
// but, in theory, will not get us reads that are as clean as the other way round.

#define COLS_ARE_OUTPUTS

// Actual hardware configuration

// ROWS
#define PORT_ROWS PORTC
#define DDR_ROWS DDRC
#define PIN_ROWS PINC
#define ROW_PINMASK  (_BV(0)|_BV(1)|_BV(2)|_BV(3))
#define ROW_COUNT 4

// COLS
#define PORT_COLS PORTD
#define DDR_COLS DDRD
#define PIN_COLS PIND
#define COL_PINMASK  (_BV(0)|_BV(1)|_BV(2)|_BV(3)|_BV(4)|_BV(5)|_BV(6)|_BV(7))
#define COL_COUNT 8


// Set data direction as output on the output pins
// Default to all output pins low

#define CONFIGURE_OUTPUT_PINS \
    PINS_HIGH(DDR_OUTPUT, OUTPUT_PINMASK); \
    PINS_LOW(PORT_OUTPUT, OUTPUT_PINMASK);

// Set the data direction for our inputs to be "input"
// Because we're reading high values, we don't want to turn on pull-ups
#define CONFIGURE_INPUT_PINS \
    PINS_LOW(DDR_INPUT, INPUT_PINMASK); \
    PINS_LOW(PORT_INPUT, INPUT_PINMASK);

#define ACTIVATE_OUTPUT_PIN(output_pin) HIGH(PORT_OUTPUT, output_pin);
#define DEACTIVATE_OUTPUT_PIN(output_pin) LOW(PORT_OUTPUT, output_pin);

	// The rows are inputs, set them back to input mode so we can read them 
	// on the next go round. By default, pullups are off, which is good because we want them off.
#define REINIT_INPUT_PINS PINS_LOW(DDR_INPUT, INPUT_PINMASK);


	// We don't have pull-down pins. Because of this, current can pretty easily leak across 
	// an entire column after a scan.
	// To pull the pins down, we flip them to outputs. By default, an output pin is driven low
	// so we don't need to explicitly drive it low.
#define CLEANUP_INPUT_PINS PINS_HIGH(DDR_INPUT, INPUT_PINMASK);

#define RECORD_KEY_STATE keyscanner_record_state_rotate_ccw();

// Active pins are high. So this macro is a no-op
#define DEBOUNCER_CANONICALIZE_PINS(pins)

#else

//Signal port (rows)
#define PORT_OUTPUT PORT_ROWS
#define DDR_OUTPUT DDR_ROWS
#define PIN_OUTPUT PIN_ROWS
#define OUTPUT_PINMASK ROW_PINMASK
#define OUTPUT_COUNT ROW_COUNT

//Scanning port (cols)
#define PORT_INPUT PORT_COLS
#define DDR_INPUT DDR_COLS
#define PIN_INPUT PIN_COLS
#define INPUT_PINMASK COL_PINMASK
#define INPUT_COUNT COL_COUNT



// Set data direction as output on the output pins
// Default to all output pins high
#define CONFIGURE_OUTPUT_PINS \
    PINS_HIGH(DDR_OUTPUT, OUTPUT_PINMASK); \
    PINS_HIGH(PORT_OUTPUT, OUTPUT_PINMASK);

// Set the data direction for our inputs to be "input"
// Turn on the pullups on the inputs
#define CONFIGURE_INPUT_PINS \
    PINS_LOW(DDR_INPUT, INPUT_PINMASK); \
    PINS_HIGH(PORT_INPUT, INPUT_PINMASK);

#define ACTIVATE_OUTPUT_PIN(output_pin) LOW(PORT_OUTPUT, output_pin);
#define DEACTIVATE_OUTPUT_PIN(output_pin) HIGH(PORT_OUTPUT, output_pin);

#define REINIT_INPUT_PINS 0;
#define CLEANUP_INPUT_PINS 0;

#define RECORD_KEY_STATE keyscanner_record_state();


// Active pins on are low. So the debouncer inverts them before working with them
#define DEBOUNCER_CANONICALIZE_PINS(pins) pins = ~pins;

#endif





// AD01: lower two bits of device address
#define AD01() ((PINB & _BV(0)) |( PINB & _BV(1)))
