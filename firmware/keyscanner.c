#include <util/delay.h>
#include "debounce.h"
#include "is31io7326.h"
#include "main.h"
#include "ringbuf.h"

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

void keyscanner_init(void)
{

    // Write to cols - we only use some of the pins in the row port
    DDR_COLS = 0xff;
    PORT_COLS = 0xff;

    // Read from rows -- We use all 8 bits of cols
    DDR_ROWS  = 0x00;
    PORT_ROWS |= ROW_PINMASK;

    // Assert comm_en so we can use the interhand transcievers
             DDRC ^= _BV(7);
             PORTC |= _BV(7);

}

static inline uint8_t popCount(uint8_t val) {
    uint8_t count;
    for (count=0; val; count++) {
        val &= val-1;
    }
    return count;
}

void keyscanner_main(void)
{
    // For each enabled row...
    // TODO: this should really draw from the ROW_PINMASK
    for (uint8_t col = 0; col < COL_COUNT; ++col) {
        // Reset all of our row pins, then unset the one we want to read as low
        PORT_COLS = (PORT_COLS | COL_PINMASK ) & ~_BV(col);
        uint8_t row_bits = PIN_ROWS;
        // Debounce key state
        uint8_t changes = debounce(row_bits, db + col);
        // Most of the time there will be no new key events
        if (__builtin_expect(changes == 0, 1)) {
     //       continue;
        }

        DISABLE_INTERRUPTS({
            state_t keystate;
            keystate.eventReported = 1;
            keystate.keyEventsWaiting = 0; // Set by I²C code (ringbuf.count != 0)
            keystate.col = 8-col;

            for (int8_t row = 0; row < 4; row++) {
                // Fewer than half the keys are expected to be down for each scanline
                if (__builtin_expect(bit_is_set(changes, row), 0)) {
                    keystate.keyState = bit_is_clear(db[col].state, row);
                    keystate.row = row;
                    ringbuf_append(keystate.val);
                }
            }

        });
    }
}
