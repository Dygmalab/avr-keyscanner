#include <util/delay.h>
#include "debounce.h"
#include "is31io7326.h"
#include "main.h"
#include "ringbuf.h"

debounce_t db[] = {
    {0x00, 0x00, 0xFF},
    {0x00, 0x00, 0xFF},
    {0x00, 0x00, 0xFF},
    {0x00, 0x00, 0xFF}
};

void keyscanner_init(void)
{

    // Write to rows - we only use some of the pins in the row port
    DDR_ROWS |= ROW_PINMASK;
    PORT_ROWS |= ROW_PINMASK;

    // Read from cols -- We use all 8 bits of cols
    DDR_COLS = 0x00;
    PORT_COLS = 0xFF;

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
    for (uint8_t row = 0; row < 4; ++row) {
        // Reset all of our row pins, then unset the one we want to read as low
        PORT_ROWS = (PORT_ROWS | ROW_PINMASK ) & ~_BV(row);


        uint8_t col_bits = PIN_COLS;
        /*
         * Rollover conditions exist if:
         *  * Multiple COLS pins are pulled low AND
         *  * Multiple ROWS pins are pulled low
         */
        /* 
        uint8_t nRows = popCount(~PIN_ROWS);
        uint8_t nCols = popCount(~col_bits);
        // Most of the time the keyboard will not be a rollover state
        if (__builtin_expect(nRows > 1 && nCols > 1, 0)) {
            continue;
        }
        */


        // Debounce key state
        uint8_t changes = debounce(col_bits, db + row);
        // Most of the time there will be no new key events
        if (__builtin_expect(changes == 0, 1)) {
            continue;
        }

        DISABLE_INTERRUPTS({
            key_t key;
            key.eventReported = 1;
            key.keyEventsWaiting = 0; // Set by I²C code (ringbuf.count != 0)
            key.row = row;

            for (int8_t col = 0; col < 8; col++) {
                // Fewer than half the keys are expected to be down for each scanline
                if (__builtin_expect(bit_is_set(changes, col), 0)) {
                    key.keyState = bit_is_clear(db[row].state, col);
                    key.col = col;
                    ringbuf_append(key.val);
                }
            }

        });
    }
}
