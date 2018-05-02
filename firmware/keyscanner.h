#pragma once

#ifdef COLS_ARE_OUTPUTS

//Scanning port (rows)
#define PORT_INPUT PORT_ROWS
#define DDR_INPUT DDR_ROWS
#define PIN_INPUT PIN_ROWS
#define MASK_INPUT MASK_ROWS
#define COUNT_INPUT COUNT_ROWS

//Signal port (cols)
#define PORT_OUTPUT PORT_COLS
#define DDR_OUTPUT DDR_COLS
#define PIN_OUTPUT PIN_COLS
#define MASK_OUTPUT MASK_COLS
#define COUNT_OUTPUT COUNT_COLS

// Set data direction as output on the output pins
// Default to all output pins low

#define CONFIGURE_OUTPUT_PINS \
    PINS_HIGH(DDR_OUTPUT, MASK_OUTPUT); \
    PINS_LOW(PORT_OUTPUT, MASK_OUTPUT);

// Set the data direction for our inputs to be "input"
// Because we're reading high values, we don't want to turn on pull-ups
#define CONFIGURE_INPUT_PINS \
    PINS_LOW(DDR_INPUT, MASK_INPUT); \
    PINS_LOW(PORT_INPUT, MASK_INPUT);

#define ACTIVATE_OUTPUT_PIN(output_pin) HIGH(PORT_OUTPUT, output_pin);
#define DEACTIVATE_OUTPUT_PIN(output_pin) LOW(PORT_OUTPUT, output_pin);

	// The rows are inputs, set them back to input mode so we can read them 
	// on the next go round. By default, pullups are off, which is good because we want them off.
#define REINIT_INPUT_PINS PINS_LOW(DDR_INPUT, MASK_INPUT);


	// We don't have pull-down pins. Because of this, current can pretty easily leak across 
	// an entire column after a scan.
	// To pull the pins down, we flip them to outputs. By default, an output pin is driven low
	// so we don't need to explicitly drive it low.
#define CLEANUP_INPUT_PINS PINS_HIGH(DDR_INPUT, MASK_INPUT);

#define RECORD_KEY_STATE keyscanner_record_state_rotate_ccw();


// When a key is pressed the input pin will read HIGH
#define KEYSCANNER_ACTIVE_HIGH

#else

//Signal port (rows)
#define PORT_OUTPUT PORT_ROWS
#define DDR_OUTPUT DDR_ROWS
#define PIN_OUTPUT PIN_ROWS
#define MASK_OUTPUT MASK_ROWS
#define COUNT_OUTPUT COUNT_ROWS

//Scanning port (cols)
#define PORT_INPUT PORT_COLS
#define DDR_INPUT DDR_COLS
#define PIN_INPUT PIN_COLS
#define MASK_INPUT MASK_COLS
#define COUNT_INPUT COUNT_COLS



// Set data direction as output on the output pins
// Default to all output pins high
#define CONFIGURE_OUTPUT_PINS \
    PINS_HIGH(DDR_OUTPUT, MASK_OUTPUT); \
    PINS_HIGH(PORT_OUTPUT, MASK_OUTPUT);

// Set the data direction for our inputs to be "input"
// Turn on the pullups on the inputs
#define CONFIGURE_INPUT_PINS \
    PINS_LOW(DDR_INPUT, MASK_INPUT); \
    PINS_HIGH(PORT_INPUT, MASK_INPUT);

#define ACTIVATE_OUTPUT_PIN(output_pin) LOW(PORT_OUTPUT, output_pin);
#define DEACTIVATE_OUTPUT_PIN(output_pin) HIGH(PORT_OUTPUT, output_pin);

#define REINIT_INPUT_PINS do {} while(0)
#define CLEANUP_INPUT_PINS do {} while(0)

#define RECORD_KEY_STATE keyscanner_record_state();

// When a key is pressed the input pin will read LOW
#undef KEYSCANNER_ACTIVE_HIGH

#endif

#ifdef KEYSCANNER_ACTIVE_HIGH

// Active pins are high. So this macro is a no-op
#define KEYSCANNER_CANONICALIZE_PINS(pins) pins

#else 

// Active pins on are low. So the debouncer inverts them before working with them
#define KEYSCANNER_CANONICALIZE_PINS(pins) ~pins

#endif


// AD01: lower two bits of device address
#define AD01() ((PINB & _BV(0)) |( PINB & _BV(1)))

void keyscanner_init(void);
void keyscanner_main(void);
void keyscanner_record_state(void);
void keyscanner_record_state_rotate_ccw(void);
void keyscanner_ringbuf_update(uint8_t row1, uint8_t row2, uint8_t row3, uint8_t row4);
void keyscanner_timer1_init(void);

void keyscanner_set_interval(uint8_t interval);
uint8_t keyscanner_get_interval();


