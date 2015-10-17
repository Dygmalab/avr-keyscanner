#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "debounce.h"
#include "is31io7326.h"
#include "main.h"
#include "ringbuf.h"
#include "twi-slave.h"

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

static inline void setup(void)
{
    DDR_PP = 0x00;
    PORT_PP = 0xFF;

    DDR_OD = 0x00;
    PORT_OD = 0xFF;

    SET_INT(1);

    // TODO: set TWI_Tx_Data_Callback and TWI_Rx_Data_Callback
    TWI_Slave_Initialise(TWI_BASE_ADDRESS | AD01());
    sei();
}

static inline void loop(void)
{
    /* TODO: low power mode:
     *   When all keys reported up:
     *     DDR_PP = 0x11; PORT_PP = 0x00;
     *     Guarantee wake on TWI / any PORT_OD pin FALLING
     *     Sleep
     */
    for (uint8_t pp = 0; pp < 8; ++pp) {
        uint8_t pp_bitmask = _BV(pp);

        _delay_ms(1);

        DDR_PP = 0x00 ^ pp_bitmask;
        PORT_PP = 0xFF ^ pp_bitmask;

        _delay_ms(1);

        // Rollover conditions exist if any other scanning pin is pulled low.
        if ((PIN_PP | pp_bitmask) != 0xFF) {
            continue;
        }

        // Read key state into debouncer
        uint8_t changes = debounce(PIN_OD, db + pp);
        if (changes == 0) {
            continue;
        }

        DISABLE_INTERRUPTS({
            key_t key;
            key.dataNumber = 0; // Set by I²C code (ringbuf.count != 0)
            key.pp = pp;

            for (int8_t od = 0; od < 8; od++) {
                if (bit_is_set(changes, od)) {
                    key.keyState = bit_is_clear(db[pp].state, od);
                    key.od = od;
                    ringbuf_append(key.val);
                }
            }

            SET_INT(0);
        });
    }
}

int main(void)
{
    setup();
    while(1){
        loop();
        SET_INT(1);
    }
    __builtin_unreachable();
}
