#pragma once
#include <stdio.h>
#include <stdint.h>
#include "keyscanner.h"

// #define DEBOUNCE_MINIMUM_MS 5
// #define KEYSCAN_INTERVAL_TO_MS_MULTIPLIER 0.032
// #define KEYSCAN_TIME  (KEYSCAN_INTERVAL  * KEYSCAN_INTERVAL_TO_MS_MULTIPLIER)
// #define DEBOUNCE_MINIMUM_CYCLES ((DEBOUNCE_MINIMUM_MS/KEYSCAN_TIME)+1)


#define PRESCALER
#define MS_TO_CYCLES(ms)  ((int) ms * (1/F_CPU ) *  KEYSCAN_INTERVAL_DEFAULT * PRESCALER


static int8_t debounce_integrator_ceiling = 18 ; // ~4ms - this means that a keypress that's 4ms long needs a 4ms gap afterward
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
    uint8_t last_state; 
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
        if (sample & _BV(i)) {
    	    if (debouncer->counters[i] < debounce_integrator_ceiling) {
                    debouncer->counters[i]++;
               	    if (debouncer->counters[i] == debounce_toggle_on_threshold &&	
    	      	    	debouncer->state ^ _BV(i) ) {
               		changes |= _BV(i);
    	    	    }
			if (debouncer->counters[i] > 2 ) 
				debouncer->counters[i] += 13;
	    }
        } else if ( debouncer->counters[i] > debounce_integrator_floor )  {
            debouncer->counters[i]--;
            if (debouncer->counters[i] == debounce_toggle_off_threshold &&
	      	debouncer->state & _BV(i) ) {
            	changes |= _BV(i);
		//debouncer->counters[i] = -18;
	    }
	    
        }

    }
    debouncer->last_state = sample;
    debouncer->state ^= changes;

    return changes;
}
