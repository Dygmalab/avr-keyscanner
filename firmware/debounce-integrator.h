#pragma once

#include <stdint.h>
#include "keyscanner.h"

// #define DEBOUNCE_MINIMUM_MS 5
// #define KEYSCAN_INTERVAL_TO_MS_MULTIPLIER 0.032
// #define KEYSCAN_TIME  (KEYSCAN_INTERVAL  * KEYSCAN_INTERVAL_TO_MS_MULTIPLIER)
// #define DEBOUNCE_MINIMUM_CYCLES ((DEBOUNCE_MINIMUM_MS/KEYSCAN_TIME)+1)


#define LOCK_OUT_CYCLES 5

static int8_t debounce_integrator_ceiling = 14 ;
static int8_t debounce_toggle_on_threshold = 4;
static int8_t debounce_integrator_floor = 0;
static int8_t debounce_toggle_off_threshold =  0;


/*
each of these 8 bit variables are storing the state for 8 keys

*/
typedef struct {
    int8_t counters[8];
    int8_t key_locked_out[8];
    uint8_t state;  // debounced state
} debounce_t;


/**
 * debounce --
 *    8 keys are processed in parallel at each operation.
 *
 * args:
 *    sample - the current state
 *    debouncer - the state variables of the debouncer
 *
 * returns: bits that have changed in the final debounced state
 *
 */
static uint8_t debounce(uint8_t sample, debounce_t *debouncer) {
    uint8_t changes = 0;

    // Scan each pin from the bank
    for(int8_t i=0; i< COUNT_INPUT; i++) {
	if (debouncer->key_locked_out[i] > 0) {
		debouncer->key_locked_out[i]--;
		continue;
	}
        if (sample & _BV(i)) {
	    if (debouncer->counters[i] < debounce_integrator_ceiling) {
                debouncer->counters[i]++;
            	if (debouncer->counters[i] == debounce_toggle_on_threshold &&	
	      	    debouncer->state ^ _BV(i) ) {
            		changes |= _BV(i);
	    	}
            } 
        } else if ( debouncer->counters[i] > debounce_integrator_floor )  {
            debouncer->counters[i]--;
            if (debouncer->counters[i] == debounce_toggle_off_threshold &&
	      	debouncer->state & _BV(i) ) {
            	changes |= _BV(i);
		debouncer->key_locked_out[i] = LOCK_OUT_CYCLES;
	    }
        }

    }
    debouncer->state ^= changes;

    return changes;
}
