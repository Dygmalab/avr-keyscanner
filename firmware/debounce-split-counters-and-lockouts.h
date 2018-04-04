#pragma once

#include <stdint.h>
#include "keyscanner.h"

/*

debounce by counting DELAY_BEFORE of stable change before registering a change.
and count DELAY_AFTER of stable change after registering a change before looking for a next one.

In other words:
- DELAY_BEFORE is the minimum latency for a key state change
- DELAY_AFTER + DELAY_BEFORE is the minimum elapsed time between 2 key state changes

(both must be minimum 1)

*/

#define DEBOUNCE_BEFORE_PRESS_DELAY_COUNT     2 // 3.2 ms
#define DEBOUNCE_AFTER_PRESS_DELAY_COUNT      6 // 9.6 ms
#define DEBOUNCE_BEFORE_RELEASE_DELAY_COUNT   6 // 9.6 ms
#define DEBOUNCE_AFTER_RELEASE_DELAY_COUNT    2 // 3.2 ms

/*
time ~= COUNT * KEYSCAN_INTERVAL * timer_prescaler * (1 / F_CPU)

- KEYSCAN_INTERVAL 50:
time ~= 10 * 50 * 256 * (1 / 8000000) = 16.0 ms
time ~= 9 * 50 * 256 * (1 / 8000000) = 14.4 ms
time ~= 8 * 50 * 256 * (1 / 8000000) = 12.8 ms
time ~= 7 * 50 * 256 * (1 / 8000000) = 11.2 ms
time ~= 6 * 50 * 256 * (1 / 8000000) = 9.6 ms
time ~= 5 * 50 * 256 * (1 / 8000000) = 8.0 ms
time ~= 4 * 50 * 256 * (1 / 8000000) = 6.4 ms
time ~= 3 * 50 * 256 * (1 / 8000000) = 4.8 ms
time ~= 2 * 50 * 256 * (1 / 8000000) = 3.2 ms
time ~= 1 * 50 * 256 * (1 / 8000000) = 1.6 ms

- KEYSCAN_INTERVAL 31:
time ~= 10 * 31 * 256 * (1 / 8000000) = 9.92 ms
time ~= 1 * 20 * 256 * (1 / 8000000) = 0.99 ms
*/


/*
each of these 8 bit variables are storing the state for 8 keys

so for key 0, the counter is represented by db0[0] and db1[0] 
and the state in state[0].
*/
typedef struct {
    int8_t counters[8];
    uint8_t lastsample;
    uint8_t state;  // debounced state
} debounce_t;

static uint8_t debounce(uint8_t sample, debounce_t *debouncer) {
    uint8_t changes = 0;
    uint8_t statechanged = sample ^ debouncer->state;
    uint8_t justchanged = sample ^ debouncer->lastsample;
    if (justchanged)
        debouncer->lastsample = sample;

    for(int8_t i=0; i< COUNT_INPUT; i++)
    {
        if (justchanged & _BV(i))
        {
            // unstable, reset stability counter
            if (debouncer->counters[i] >= 0) // begin/reset counter "BEFORE" a change
            {
                if (debouncer->state & _BV(i)) // registered state is pressed, so key is releasing
                    debouncer->counters[i] = DEBOUNCE_BEFORE_RELEASE_DELAY_COUNT - 1;
                else
                    debouncer->counters[i] = DEBOUNCE_BEFORE_PRESS_DELAY_COUNT - 1;
            }
            else // still within the "AFTER" delay
            {
                if (debouncer->state & _BV(i)) // registered state is pressed
                    debouncer->counters[i] = -(DEBOUNCE_AFTER_PRESS_DELAY_COUNT - 1);
                else
                    debouncer->counters[i] = -(DEBOUNCE_AFTER_RELEASE_DELAY_COUNT - 1);
            }
        }
        else if (debouncer->counters[i] != 0)
        {
            // stabilizing, converge to 0
            debouncer->counters[i] -= debouncer->counters[i] > 0 ? 1 : -1;
        }
        else if (statechanged & _BV(i))
        {
            if (debouncer->state & _BV(i)) // last registered state was pressed, so key is releasing
                debouncer->counters[i] = -(DEBOUNCE_AFTER_RELEASE_DELAY_COUNT - 1);
            else
                debouncer->counters[i] = -(DEBOUNCE_AFTER_PRESS_DELAY_COUNT - 1);
            debouncer->state ^= _BV(i);
            changes |= _BV(i);
        }
    }
    return changes;
}
