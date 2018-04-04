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
#define MASK_ROWS  (_BV(0)|_BV(1)|_BV(2)|_BV(3))
#define COUNT_ROWS 4

// COLS
#define PORT_COLS PORTD
#define DDR_COLS DDRD
#define PIN_COLS PIND
#define MASK_COLS  (_BV(0)|_BV(1)|_BV(2)|_BV(3)|_BV(4)|_BV(5)|_BV(6)|_BV(7))
#define COUNT_COLS 8


// AD01: lower two bits of device address
#define AD01() ((PINB & _BV(0)) |( PINB & _BV(1)))
