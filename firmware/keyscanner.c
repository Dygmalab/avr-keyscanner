#include <util/delay.h>
#include "debounce.h"
#include "wire-protocol.h"
#include "main.h"
#include "ringbuf.h"

debounce_t db[] = {
    {0x00, 0x00, 0xFF},
    {0x00, 0x00, 0xFF},
    {0x00, 0x00, 0xFF},
    {0x00, 0x00, 0xFF}
};

void keyscanner_init(void) {

    // Write to rows - we only use some of the pins in the row port
    DDR_ROWS = 0xff;
    PORT_ROWS = ROW_PINMASK;

    // Read from cols -- We use all 8 bits of cols
    DDR_COLS  = 0x00;
    PORT_COLS = 0xFF;

    // Assert comm_en so we can use the interhand transcievers
    // (Until comm_en on the i2c transcievers is pulled high,
    //  they're disabled)
//    DDRC ^= _BV(7);
    // PC7 is on the same port as the four row pins.
    // We refer to it here as PORTC because
    // we're not using it as part of the keyscanner
    HIGH(PORTC,7);

}

void keyscanner_main(void) {
    uint8_t debounced_changes = 0;
    uint8_t pin_data;

    // For each enabled row...
    for (uint8_t row = 0; row < ROW_COUNT; ++row) {
        // Reset all of our row pins, then
        // set the one we want to read as low
        PORT_ROWS = (PORT_ROWS | ROW_PINMASK ) & ~_BV(row);
        pin_data = PIN_COLS;
        // Debounce key state
        debounced_changes += debounce((pin_data) , db + row);
    }


    // Most of the time there will be no new key events
    if (__builtin_expect(debounced_changes == 0, 1)) {
        // Use an internal delay funciton Because the toplevel _delay_us function wants a
        // compile time constant.
        //
        // At 8MHz, a value of 2 gets us 1 microsecond of delay
        // Only run the debouncing delay when we haven't successfully found
        // a debounced event
        _delay_loop_2(debounce_delay);
        return;
    }

    // Snapshot the keystate to add to the ring buffer
    DISABLE_INTERRUPTS({
        // The fact that we're sticking rows 1,2,3,0 into the ringbuffer
        // rather than 0,1,2,3 in is due to some bug Jesse hasn't tracked
        // down yet.
        ringbuf_append( db[1].state ^ 0xff );
        ringbuf_append( db[2].state ^ 0xff );
        ringbuf_append( db[3].state ^ 0xff );
        ringbuf_append( db[0].state ^ 0xff );
    });
}
