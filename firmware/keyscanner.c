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
    DDR_ROWS = 0x00;
    PORT_ROWS = 0xFF;

    DDR_COLS = 0x00;
    PORT_COLS = 0xFF;
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
    /* TODO: low power mode:
     *   When all keys reported up:
     *     DDR_PP = 0x11; PORT_PP = 0x00;
     *     Guarantee wake on TWI / any PORT_OD pin FALLING
     *     Sleep
     */

    // For each enabled row...
    for (uint8_t row = 0; row < 4; ++row) {
        uint8_t row_bitmask = _BV(pp);

	/* HACK: exclude SPI pins from the DDR/PORT mask */
        DDR_ROWS = (0x00 ^ row_bitmask) | ( _BV(5)|_BV(3)|_BV(2));
        PORT_ROWS = (0xFF ^ row_bitmask) & ~(_BV(5)|_BV(3)|_BV(2));


        uint8_t col_bits = PIN_COLS;
        /*
         * Rollover conditions exist if:
         *  * Multiple COLS pins are pulled low AND
         *  * Multiple ROWS pins are pulled low
         */
        uint8_t nRows = popCount(~PIN_ROWS);
        uint8_t nCols = popCount(~col_bits);
        // Most of the time the keyboard will not be a rollover state
        if (__builtin_expect(nRows > 1 && nCols > 1, 0)) {
            continue;
        }

        // Debounce key state
        uint8_t changes = debounce(col_bits, db + row);
        // Most of the time there will be no new key events
        if (__builtin_expect(changes == 0, 1)) {
            continue;
        }

        DISABLE_INTERRUPTS({
            key_t key;
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
