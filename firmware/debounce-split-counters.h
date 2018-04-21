#pragma once

#include <stdint.h>
#include "keyscanner.h"

/**
 * original "debounce-counter" algo but with customizable press and release
 * delay (at compile-time).
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
 *
 *   KEYSCAN_INTERVAL=14:
 *
 *     delay ~= 30 * 14 * 256 * (1 / 8000000) = 13.4 ms
 *     delay ~= 29 * 14 * 256 * (1 / 8000000) = 13 ms
 *     delay ~= 28 * 14 * 256 * (1 / 8000000) = 12.5 ms
 *     delay ~= 27 * 14 * 256 * (1 / 8000000) = 12.1 ms
 *     delay ~= 26 * 14 * 256 * (1 / 8000000) = 11.6 ms
 *     delay ~= 25 * 14 * 256 * (1 / 8000000) = 11.2 ms
 *     delay ~= 24 * 14 * 256 * (1 / 8000000) = 10.8 ms
 *     delay ~= 23 * 14 * 256 * (1 / 8000000) = 10.3 ms
 *     delay ~= 22 * 14 * 256 * (1 / 8000000) = 9.9 ms
 *     delay ~= 21 * 14 * 256 * (1 / 8000000) = 9.4 ms
 *     delay ~= 20 * 14 * 256 * (1 / 8000000) = 9 ms
 *     delay ~= 19 * 14 * 256 * (1 / 8000000) = 8.5 ms
 *     delay ~= 18 * 14 * 256 * (1 / 8000000) = 8.1 ms
 *     delay ~= 17 * 14 * 256 * (1 / 8000000) = 7.6 ms
 *     delay ~= 16 * 14 * 256 * (1 / 8000000) = 7.2 ms
 *     delay ~= 15 * 14 * 256 * (1 / 8000000) = 6.7 ms
 *     delay ~= 14 * 14 * 256 * (1 / 8000000) = 6.3 ms
 *     delay ~= 13 * 14 * 256 * (1 / 8000000) = 5.8 ms
 *     delay ~= 12 * 14 * 256 * (1 / 8000000) = 5.4 ms
 *     delay ~= 11 * 14 * 256 * (1 / 8000000) = 4.9 ms
 *     delay ~= 10 * 14 * 256 * (1 / 8000000) = 4.5 ms
 *     delay ~= 9 * 14 * 256 * (1 / 8000000) = 4 ms
 *     delay ~= 8 * 14 * 256 * (1 / 8000000) = 3.6 ms
 *     delay ~= 7 * 14 * 256 * (1 / 8000000) = 3.1 ms
 *     delay ~= 6 * 14 * 256 * (1 / 8000000) = 2.7 ms
 *     delay ~= 5 * 14 * 256 * (1 / 8000000) = 2.2 ms
 *     delay ~= 4 * 14 * 256 * (1 / 8000000) = 1.8 ms
 *     delay ~= 3 * 14 * 256 * (1 / 8000000) = 1.3 ms
 *     delay ~= 2 * 14 * 256 * (1 / 8000000) = 0.9 ms
 *     delay ~= 1 * 14 * 256 * (1 / 8000000) = 0.4 ms
 *
 */


#define DEBOUNCE_PRESS_DELAY_COUNT 2
#define DEBOUNCE_RELEASE_DELAY_COUNT 50

/*
 * like the original debounce-counter, counters are transposed:
 * - instead of counters[pin_bit] = counter_bits
 * - we store counter_bits[counter_bit] = pin_bits
 *
 * When optimized, it should generates a small branch-less code.
 * higher delays will generate more code.
 *
 * // -O3, __attribute__ ((noinline)) uint8_t debounce()
 * debouncer             instruction count
 * none                   5
 * counter               18
 * integrator            72
 * old split-counter     94
 * old split-counter     44 (branchless "transposed" version)
 * split-counter 0 0      5 (no delay)
 * split-counter 3 3     18 (same delays as original debounce-counter)
 * split-counter 2 6     37
 * split-counter 3 7     34
 * split-counter 4 8     44
 * split-counter 4 9     48
 * split-counter 253 253 72 (still branchless (unrolled), uses a counters[8])
 *
 * none, counter, and split-counter are branchless, so performance should
 * be grossly proportional to their code size.
 *
 * integrator, and old split-counter have branches and potentially loops
 * still in their final assembly, so performance is not related to code size.
 */
#define _MAX(a, b) ((b) > (a) ? (b) : (a))
// old compilers can't do clz at compile time (avr gcc 4.6.4)
//#define _NUM_BITS(x) (sizeof(int) * 8 - __builtin_clz(x))
#define _NUM_BITS(x) ((x)<1?0:(x)<2?1:(x)<4?2:(x)<8?3:(x)<16?4:(x)<32?5:(x)<64?6:(x)<128?7:(x)<256?8:-1)
#define NUM_COUNTER_BITS _NUM_BITS(_MAX(DEBOUNCE_RELEASE_DELAY_COUNT, DEBOUNCE_PRESS_DELAY_COUNT))

/*
 * _DEBOUCE_FORCE_RESET forces a counter reset each time the state changes.
 *
 * without _DEBOUCE_FORCE_RESET's code, when delays are different or a `delay+1`
 * is not a power two, and when the key sample changes again just after we
 * register a state change, then the counter is not reset.
 * (e.g. {state,sample} changes from {0,1} to {1,0}, then state_changed is 1
 * both times).
 * (does not matter with same `delay+1` power of two, because counter overflows
 * to zero)
 *
 * so _DEBOUCE_FORCE_RESET fix this by introducing an additional lockout delay of
 * 1 just after a state change to reset to counter.
 */
#define _IS_POWER_OF_TWO(x) (((x) & ((x) - 1)) == 0)
#define _DEBOUNCE_FORCE_RESET (DEBOUNCE_PRESS_DELAY_COUNT != DEBOUNCE_RELEASE_DELAY_COUNT || \
                               !_IS_POWER_OF_TWO(DEBOUNCE_PRESS_DELAY_COUNT+1) || \
                               !_IS_POWER_OF_TWO(DEBOUNCE_RELEASE_DELAY_COUNT+1))

typedef struct {
    uint8_t counter_bits[NUM_COUNTER_BITS];
    uint8_t last_changes;
    uint8_t state;  // debounced state
} debounce_t;

__attribute__((optimize("unroll-loops"))) // we want to unroll loops, even when "only" -O2
//__attribute__ ((noinline))
static inline
uint8_t debounce(uint8_t sample, debounce_t *debouncer) {

    // this code is optimized for and by the compiler's unroll-loops, and it
    // tries to give the compiler the opportunity to generate as little
    // instructions as possible.
    //
    // so, when given the same delays as the hard-coded 'debounce-counter'
    // (release=3, press=3), this code should generate the same number of
    // instructions (and same behavior).

    uint8_t state_changed = sample ^ debouncer->state;
    uint8_t carry_inc = ~0;
    uint8_t waited_for_press_delay = ~0;
    uint8_t waited_for_release_delay = ~0;

    if (_DEBOUNCE_FORCE_RESET != 0)
        // if state changed during last debounce call, then force state_changed
        // to 0 this time, so the counter has a chance to be reset.
        state_changed &= ~debouncer->last_changes;

    // foreach bit in counter_bits
    for(uint8_t i=0; i<NUM_COUNTER_BITS; i++)
    {
        // increment the counter if state changed, else reset the counter to zero.
        // after simplification, we increment by flipping the bits one by one
        // stopping after getting a 1.
        debouncer->counter_bits[i] = (debouncer->counter_bits[i] ^ carry_inc) & state_changed;
        carry_inc &= ~debouncer->counter_bits[i];
        // note:
        //   the carry is this way (and not, for example `counter ^= ~carry_inc;
        //   carry_inc |= counter`) because it seems to help the compiler reuse
        //   it for waited_for_*_delay when the firsts delay_plus_one_bit are 0
        //   (for example, when delays=(3, 3), at the end `waited_for_*_delay =
        //   carry_inc`)

        // test if we waited for DELAY.
        // notes:
        // - (1) we only need to compare up to the number of bits of `DELAY`.
        // - (2) here, the counter is already incremented to 1 at the first
        //       state_changed (see above).
        // - (3) to get the right delay we actually need to compare to `DELAY + 1`
        //       (because of (2)).
        // - (4) because of (1) and (3), delays like `3` will overflow to
        //       `bits[2] = {0, 0}`.
        // - (5) but we don't mind the overflow (4) (we don't need to add a bit
        //       to counter_bits, and (1) still works) because we can use the
        //       overflow to zero `{0, 0}` (4) as a valid counter value because
        //       of (2).
        if (i < _NUM_BITS(DEBOUNCE_PRESS_DELAY_COUNT))
            waited_for_press_delay &= (
                ((DEBOUNCE_PRESS_DELAY_COUNT + 1) & _BV(i)) ?
                debouncer->counter_bits[i] :
                ~debouncer->counter_bits[i]);

        // ditto
        if (i < _NUM_BITS(DEBOUNCE_RELEASE_DELAY_COUNT))
            waited_for_release_delay &= (
                ((DEBOUNCE_RELEASE_DELAY_COUNT + 1) & _BV(i)) ?
                debouncer->counter_bits[i] :
                ~debouncer->counter_bits[i]);
    }

    // change key state if state_changed and we waited for press or release delay
    uint8_t changes = state_changed & ((~debouncer->state & waited_for_press_delay) |
                                       ( debouncer->state & waited_for_release_delay));
    if (_DEBOUNCE_FORCE_RESET != 0)
        debouncer->last_changes = changes;
    debouncer->state ^= changes;
    return changes;
}
