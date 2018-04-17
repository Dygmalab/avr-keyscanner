#pragma once

#include <stdint.h>
#include "keyscanner.h"

// #define DEBOUNCE_MINIMUM_MS 5
// #define KEYSCAN_INTERVAL_TO_MS_MULTIPLIER 0.032
// #define KEYSCAN_TIME  (KEYSCAN_INTERVAL  * KEYSCAN_INTERVAL_TO_MS_MULTIPLIER)
// #define DEBOUNCE_MINIMUM_CYCLES ((DEBOUNCE_MINIMUM_MS/KEYSCAN_TIME)+1)


#define PRESCALER
#define MS_TO_CYCLES(ms)  ((int) ms * (1/F_CPU ) *  KEYSCAN_INTERVAL_DEFAULT * PRESCALER

#define LOCK_OUT_CYCLES 0

static int8_t debounce_integrator_ceiling = 8 ; // ~4ms - this means that a keypress that's 4ms long needs a 4ms gap afterward
static int8_t debounce_toggle_on_threshold = 3; // ~1ms
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



//  If a key has been on for two cycles (1ms) but less than 12 cycles (5ms) then 
//      	for the next (10 cycles) 5ms, we treat every scan as ON.
//  
//  If a key has toggled off
//  	treat any event in the next 5ms (10 cycles) as "off"
//  


static uint8_t debounce(uint8_t sample, debounce_t *debouncer) {
    uint8_t changes = 0;

    // Scan each pin from the bank
    for(int8_t i=0; i< COUNT_INPUT; i++) {
	if (debouncer->key_locked_out[i] > 0) {
		// If the lockout is running, then force the counter to keep running, even if it goes outside the limits
		debouncer->key_locked_out[i]--;
		// If, during the lockout period we get even a single jitter
		// of 'same state as before', it means there's chatter, so we 
		// extend the lockout period
		if ( !(sample & _BV(i))  &&  (debouncer->state & _BV(i))) {
    				debouncer->key_locked_out[i] += 1; //LOCK_OUT_CYCLES;

			
		}
    		(debouncer->state & _BV(i)) ? debouncer->counters[i]++ : debouncer->counters[i]--;
		continue;
	}
        if (sample & _BV(i)) {
    	    if (debouncer->counters[i] < debounce_integrator_ceiling) {
                    debouncer->counters[i]++;
               	    if (debouncer->counters[i] == debounce_toggle_on_threshold &&	
    	      	    	debouncer->state ^ _BV(i) ) {
    				debouncer->key_locked_out[i] = 10; //LOCK_OUT_CYCLES;
               			changes |= _BV(i);
    	    		}
	    }
        } else if ( debouncer->counters[i] > debounce_integrator_floor )  {
            debouncer->counters[i]--;
            if (debouncer->counters[i] == debounce_toggle_off_threshold &&
	      	debouncer->state & _BV(i) ) {
		debouncer->key_locked_out[i] =2 ; //LOCK_OUT_CYCLES;
            	changes |= _BV(i);
	    }
        }

    }
    debouncer->state ^= changes;

    return changes;
}
