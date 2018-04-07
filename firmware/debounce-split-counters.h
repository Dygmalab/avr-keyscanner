#pragma once

#include <stdint.h>
#include "keyscanner.h"

/**
 * original "debounce-counter" algo but with customizable press and
 * release delay (at compile-time).
 *
 * debounces by waiting for a change to last DELAY before registering it.
 *
 * So,
 * - PRESS_DELAY is the minimum latency for a key press
 * - RELEASE_DELAY is the minimum latency for a key release, or, in other words,
 *   it's the minimum elapsed time a key is pressed.
 *
 * delay cheat sheet:
 *   delay ~= DELAY_COUNT * KEYSCAN_INTERVAL * timer_prescaler * (1 / F_CPU)
 *   KEYSCAN_INTERVAL=50:
 *     delay ~= 12 * 50 * 256 * (1 / 8000000) = 19.2 ms
 *     delay ~= 11 * 50 * 256 * (1 / 8000000) = 17.6 ms
 *     delay ~= 10 * 50 * 256 * (1 / 8000000) = 16.0 ms
 *     delay ~= 9 * 50 * 256 * (1 / 8000000) = 14.4 ms
 *     delay ~= 8 * 50 * 256 * (1 / 8000000) = 12.8 ms
 *     delay ~= 7 * 50 * 256 * (1 / 8000000) = 11.2 ms
 *     delay ~= 6 * 50 * 256 * (1 / 8000000) = 9.6 ms
 *     delay ~= 5 * 50 * 256 * (1 / 8000000) = 8.0 ms
 *     delay ~= 4 * 50 * 256 * (1 / 8000000) = 6.4 ms
 *     delay ~= 3 * 50 * 256 * (1 / 8000000) = 4.8 ms
 *     delay ~= 2 * 50 * 256 * (1 / 8000000) = 3.2 ms
 *     delay ~= 1 * 50 * 256 * (1 / 8000000) = 1.6 ms
 *
 */
#define DEBOUNCE_PRESS_DELAY_COUNT 3 // 4.8 ms
#define DEBOUNCE_RELEASE_DELAY_COUNT 7 // 11.2 ms

/*
 * like the original debounce-counter, counters are tranposed:
 * - instead of counters[pin_bit] = counter_bits
 * - we store counterbits[counter_bit] = pin_bits
 *
 * When optimzed, it should generates a small branch-less code.
 * higher delays will generate more code.
 *
 * // -O3, __attribute__ ((noinline)) uint8_t debounce()
 * deboucer              instruction count
 * none                   5
 * counter               18
 * integrator            72
 * old split-counter     94
 * old split-counter     44 (branchless "transposed" version)
 * split-counter 0 0      5 (no delay)
 * split-counter 3 3     18 (same delays as original debounce-counter)
 * split-counter 3 7     31
 * split-counter 4 9     49
 * split-counter 250 250 72 (uses a counters[8])
 *
 * none, counter, and split-counter are branchless, so performance should
 * be grossly proportional to their code size.
 *
 * integrator, and old split-counter have branches and potentially loops
 * still in their final assembly, so performance is not related to code size.
 */
#define _MAX(a, b) ((b) > (a) ? (b) : (a))
// old compilers can't do clz at compile time
//#define _NUM_BITS(x) (sizeof(int) * 8 - __builtin_clz(x))
#define _NUM_BITS(x) ((x)<2?1:(x)<4?2:(x)<8?3:(x)<16?4:(x)<32?5:(x)<64?6:(x)<128?7:(x)<256?8:-1)
#define NUM_COUNTER_BITS _NUM_BITS(_MAX(DEBOUNCE_RELEASE_DELAY_COUNT, DEBOUNCE_PRESS_DELAY_COUNT))

typedef struct {
    uint8_t counterbits[NUM_COUNTER_BITS];
    uint8_t state;  // debounced state
} debounce_t;

__attribute__((optimize("unroll-loops"))) // we want to unroll loops, even when "only" -O2
//__attribute__ ((noinline))
static inline
uint8_t debounce(uint8_t sample, debounce_t *debouncer) {
    uint8_t statechanged = sample ^ debouncer->state;
    uint8_t c = ~0;
    uint8_t hit_press_delay = ~0;
    uint8_t hit_release_delay = ~0;

    // foreach bit in counterbits
    for(uint8_t i=0; i<NUM_COUNTER_BITS; i++)
    {
        // increment if state changed, else reset to zero
        debouncer->counterbits[i] = (debouncer->counterbits[i] ^ c) & statechanged;
        c &= ~debouncer->counterbits[i]; // kind of carry
        // compare counterbits with DELAY bits
        if (i < _NUM_BITS(DEBOUNCE_PRESS_DELAY_COUNT))
            hit_press_delay &= (((DEBOUNCE_PRESS_DELAY_COUNT + 1) & _BV(i)) ? debouncer->counterbits[i] : ~debouncer->counterbits[i]);
        if (i < _NUM_BITS(DEBOUNCE_RELEASE_DELAY_COUNT))
            hit_release_delay &= (((DEBOUNCE_RELEASE_DELAY_COUNT + 1) & _BV(i)) ? debouncer->counterbits[i] : ~debouncer->counterbits[i]);
    }
    uint8_t changes = statechanged & ((debouncer->state & hit_release_delay) | (~debouncer->state & hit_press_delay));
    debouncer->state ^= changes;
    return changes;
}
