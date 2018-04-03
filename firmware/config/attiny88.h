#pragma once

/*
 * Application considerations:
 * The ATtiny48/88 has three 8-bit ports available.
 * Two have special functionality of interest to the application:
 *     PORTB: used by ISP
 *     PORTC: used by TWI/I²C
 *
 */

// ROWS: Signal port (rows)
#define PORT_ROWS PORTC
#define DDR_ROWS DDRC
#define PIN_ROWS PINC

#define ROW_PINMASK  (_BV(0)|_BV(1)|_BV(2)|_BV(3))

#define ROW_COUNT 4

// COLS: Scanning port (cols)
#define PORT_COLS PORTD
#define DDR_COLS DDRD
#define PIN_COLS PIND

#define COL_PINMASK  0xFF
#define COL_COUNT 8


#define PORT_INPUT PORT_ROWS
#define DDR_INPUT DDR_ROWS
#define PIN_INPUT PIN_ROWS
#define INPUT_PINMASK ROW_PINMASK
#define INPUT_COUNT ROW_COUNT

#define PORT_OUTPUT PORT_COLS
#define DDR_OUTPUT DDR_COLS
#define PIN_OUTPUT PIN_COLS
#define OUTPUT_PINMASK COL_PINMASK
#define OUTPUT_COUNT COL_COUNT


// AD01: lower two bits of device address
#define AD01() ((PINB & _BV(0)) |( PINB & _BV(1)))
