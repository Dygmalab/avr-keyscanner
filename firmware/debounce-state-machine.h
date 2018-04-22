#pragma once
#include <stdio.h>
#include <stdint.h>
#include "keyscanner.h"

#define TURNING_ON_CHATTER_WINDOW 2
#define LOCKED_ON_PERIOD 11
#define KEY_ON_CHATTER_WINDOW  5
#define TURNING_OFF_CHATTER_WINDOW 3
#define LOCKED_OFF_PERIOD 34

# define BAD_SWITCH_TURNING_ON_CHATTER_WINDOW 2
# define BAD_SWITCH_LOCKED_ON_PERIOD 22
# define BAD_SWITCH_KEY_ON_CHATTER_WINDOW  10
# define BAD_SWITCH_TURNING_OFF_CHATTER_WINDOW 35
# define BAD_SWITCH_LOCKED_OFF_PERIOD 50


#define CHATTER_DETECTED 1
#define NO_CHATTER_DETECTED 0

#define debug(x)  //printf(x); printf("\n");


enum { OFF, TURNING_ON, LOCKED_ON, ON, TURNING_OFF, LOCKED_OFF};


// these timers are in the same order as the enum above;
//
uint8_t regular_timers[] =           { 0,  2, 11,   5,   3,   34 };
uint8_t chattering_switch_timers[] = { 0,  2, 35,  10,  38,   50 };

typedef struct {
    uint8_t key_states[8];
    uint8_t cycles[8];
    uint8_t per_state_data[8];
    uint8_t key_chatters[8];
    uint8_t state;  // debounced state
} debounce_t;



uint8_t transition_to_state(debounce_t *debouncer, uint8_t i, uint8_t new_state) {
    debouncer->key_states[i]= new_state;
    debouncer->cycles[i]=0;
    debouncer->per_state_data[i]=(debouncer->key_chatters[i] ? chattering_switch_timers[new_state]: regular_timers[new_state] );
    return new_state;
}


void chatter_detected ( debounce_t *debouncer, uint8_t i) {
    debouncer->key_chatters[i]=1;
    debouncer->per_state_data[i]= chattering_switch_timers[debouncer->key_states[i]];

}

int8_t handle_state_off (uint8_t is_on, uint8_t i, debounce_t *debouncer) {
    if (!is_on) {
        return transition_to_state(debouncer,i, OFF);
    }

    // if we get a single input sample that's "1", transition to "TURNING_ON".
    if ( debouncer->cycles[i] > debouncer->per_state_data[i] ) {
        return transition_to_state(debouncer, i, TURNING_ON);
    }
    return debouncer->key_states[i];
}

int8_t handle_state_on(uint8_t is_on, uint8_t i, debounce_t *debouncer) {
    if (is_on) {
        return transition_to_state(debouncer,i, ON);
    }

    // 	if all of the last 10ms of samples are "0", transition to "TURNING_OFF"
    if ( debouncer->cycles[i] > debouncer->per_state_data[i] ) {
        return transition_to_state(debouncer, i, TURNING_OFF);
    }
    return debouncer->key_states[i];
}

int8_t handle_state_turning_on(uint8_t is_on, uint8_t i, debounce_t *debouncer) {
    if (!is_on)  {
        chatter_detected(debouncer,i);
        return transition_to_state(debouncer,i, OFF);
    }

    if ( debouncer->cycles[i] > debouncer->per_state_data[i] ) {
    	return transition_to_state(debouncer,i, LOCKED_ON);
    }

    return debouncer->key_states[i];
}


int8_t handle_state_turning_off(uint8_t is_on, uint8_t i, debounce_t *debouncer) {
    if(is_on) {
	chatter_detected(debouncer, i);
        return transition_to_state(debouncer, i, ON);
    }

    if( debouncer->cycles[i] > debouncer->per_state_data[i] ) {
        return transition_to_state(debouncer, i, LOCKED_OFF);
    }
    return debouncer->key_states[i];
}

int8_t handle_state_locked_on(uint8_t is_on, uint8_t i, debounce_t *debouncer) {
    if (!is_on) {
	chatter_detected(debouncer,i);
    }

    // 	do not act on any input while the key is locked on
    if(debouncer->cycles[i] > debouncer->per_state_data[i]) {
        return transition_to_state(debouncer,i, ON);
    }
    return debouncer->key_states[i];
}


int8_t handle_state_locked_off(uint8_t is_on, uint8_t i, debounce_t *debouncer) {
    // 	do not act on any input during the locked off window
    if (is_on) {
        chatter_detected(debouncer, i);
        return transition_to_state(debouncer, i, LOCKED_OFF);
    }

    // 	after 45ms transition to "OFF"
    if(debouncer->cycles[i] > debouncer->per_state_data[i] ) {
	return transition_to_state(debouncer, i, OFF);
    }

    return debouncer->key_states[i];
}



static uint8_t debounce(uint8_t sample, debounce_t *debouncer) {
    uint8_t changes = 0;
    // Scan each pin from the bank
    for(int8_t i=0; i< COUNT_INPUT; i++) {
        uint8_t is_on=  !! (sample & _BV(i)) ;
        debouncer->cycles[i]++;

        switch (debouncer->key_states[i] ) {
        case OFF:
            handle_state_off(is_on, i, debouncer);
            break;

        case TURNING_ON:
            if (handle_state_turning_on(is_on, i, debouncer) == LOCKED_ON) {
    		changes |= _BV(i);
	    }
            break;

        case LOCKED_ON:
            handle_state_locked_on(is_on, i, debouncer);
            break;

        case ON:
            handle_state_on(is_on, i, debouncer);
            break;

        case TURNING_OFF:
            if (handle_state_turning_off(is_on, i, debouncer) == LOCKED_OFF) {
    		changes |= _BV(i);
	    }
            break;

        case LOCKED_OFF:
            handle_state_locked_off(is_on, i, debouncer);
            break;
        };

    }


    debouncer->state ^= changes;

    return changes;
}
