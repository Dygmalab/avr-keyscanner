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

// COLS: Scanning port (cols) 
#define PORT_COLS PORTD
#define DDR_COLS DDRD
#define PIN_COLS PIND

// AD01: lower two bits of device address
#define AD01() ((PINB & _BV(0)) |( PINB & _BV(1)))
