#pragma once

#include <string.h>
#include <stdint.h>

/*

debounce by waiting for a DELAY with no change before registering the change.

- PRESS_DELAY is the minimum latency for a key press
- RELEASE_DELAY is the minimum latency for a key release, or, in other words,
  it's the minimum elapsed time a key is pressed.

(both must be minimum 1)

*/

#define DEBOUNCE_PRESS_DELAY_COUNT      2 // 3.2 ms
#define DEBOUNCE_RELEASE_DELAY_COUNT    7 // 9.6 ms
//#define DEBOUNCE_RELEASE_DELAY_COUNT    12 // 19.2 ms

/*
time ~= COUNT * KEYSCAN_INTERVAL * timer_prescaler * (1 / F_CPU)

- KEYSCAN_INTERVAL 50:
time ~= 12 * 50 * 256 * (1 / 8000000) = 19.2 ms
time ~= 11 * 50 * 256 * (1 / 8000000) = 17.6 ms
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
    uint8_t counters[8];
    uint8_t lastsample;
    uint8_t state;  // debounced state
} debounce_t;


inline void debouncer_init (debounce_t *db, uint8_t count){
    for (int i = 0; i< count; i++) {
        db[i].state = 0x00;
        db[i].lastsample = 0x00;
        memset(db[i].counters,0,sizeof db[i].counters);
    }
}


static uint8_t debounce(uint8_t sample, debounce_t *debouncer) {
    sample = ~sample; // pin HIGH == key NOT PRESSED reverse now for easier code readability (and final state)

    uint8_t changes = 0;
    uint8_t statechanged = sample ^ debouncer->state;
    uint8_t justchanged = sample ^ debouncer->lastsample;
    if (justchanged)
        debouncer->lastsample = sample;

    for(int8_t i=0; i< INPUT_COUNT; i++)
    {
        if (justchanged & _BV(i))
        {
            // unstable, reset counter
            if (debouncer->state & _BV(i))
                debouncer->counters[i] = DEBOUNCE_RELEASE_DELAY_COUNT - 1;
            else
                debouncer->counters[i] = DEBOUNCE_PRESS_DELAY_COUNT - 1;
        }
        else if (debouncer->counters[i] != 0)
        {
            // stabilizing, count down
            debouncer->counters[i]--;
        }
        else if (statechanged & _BV(i))
        {
            // stabilized, register the change
            debouncer->state ^= _BV(i);
            changes |= _BV(i);
        }
    }
    return changes;
}
