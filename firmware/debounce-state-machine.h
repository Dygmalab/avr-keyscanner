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


#define EXPECTED_OFF 0
#define EXPECTED_ON  1

enum { OFF, TURNING_ON, LOCKED_ON, ON, TURNING_OFF, LOCKED_OFF};

int next_lifecycle[] = { TURNING_ON, LOCKED_ON, ON, TURNING_OFF, LOCKED_OFF, OFF};

int in_event_of_chatter[] = { OFF, OFF , -1, ON, ON, -1 }; 
// these timers are in the same order as the enum above;
//
uint8_t regular_timers[] =           { 0,  1, 7,   10,   6,   30 };
uint8_t chattering_switch_timers[] = { 0,  2, 35,  65,  27,   50 };

typedef struct {
    uint8_t lifecycle;
    uint8_t ticks;
    uint8_t timer;
    uint8_t chatters;
} key_info_t;

typedef struct {
    key_info_t key_info[8];
    uint8_t state;  // debounced state
} debounce_t;



uint8_t transition_to_state(key_info_t *key_info, int8_t new_state) {
    if (new_state == -1) {
	    return key_info->lifecycle;
    }
    key_info->lifecycle= new_state;
    key_info->ticks=0;
    key_info->timer=(key_info->chatters ? chattering_switch_timers[new_state]: regular_timers[new_state] );
    return new_state;
}


void chatter_detected ( key_info_t *key_info) {
    key_info->chatters=1;
    key_info->timer= chattering_switch_timers[key_info->lifecycle];

}

int8_t handle_state_steady(uint8_t is_on, uint8_t expected_signal, key_info_t *key_info) {
    if (is_on != expected_signal) {
        transition_to_state(key_info, key_info->lifecycle);
    }

    if ( key_info->ticks > key_info->timer ) {
        transition_to_state(key_info, next_lifecycle[key_info->lifecycle]);
    }
    return key_info->lifecycle;
}

int8_t handle_state_turning_x(uint8_t is_on, uint8_t expected_signal, key_info_t *key_info) {
    if(is_on != expected_signal) {
        chatter_detected(key_info);
        transition_to_state(key_info, in_event_of_chatter[key_info->lifecycle]);
    }

    if( key_info->ticks > key_info->timer ) {
        transition_to_state(key_info,  next_lifecycle[key_info->lifecycle]);
    }
    return key_info->lifecycle;
}

int8_t handle_state_locked(uint8_t is_on, uint8_t expected_signal, key_info_t *key_info) { 

    // if we get the 'other' value during a locked window, that's gotta be chatter
    if (is_on != expected_signal) {
        chatter_detected(key_info);
        transition_to_state(key_info, in_event_of_chatter[key_info->lifecycle]);
    }

    // 	do not act on any input during the locked off window
    if(key_info->ticks > key_info->timer) {
        transition_to_state(key_info, next_lifecycle[key_info->lifecycle]);
    }
    return key_info->lifecycle;
}




static uint8_t debounce(uint8_t sample, debounce_t *debouncer) {
    uint8_t changes = 0;
    // Scan each pin from the bank
    for(int8_t i=0; i< COUNT_INPUT; i++) {
        uint8_t is_on=  !! (sample & _BV(i)) ;
        key_info_t *key_info	= debouncer->key_info+i;

        key_info->ticks++;

        switch (key_info->lifecycle ) {
        case OFF:
            handle_state_steady(is_on, EXPECTED_ON, key_info);
            break;

        case TURNING_ON:
            if (handle_state_turning_x(is_on, EXPECTED_ON, key_info) == LOCKED_ON) {
                changes |= _BV(i);
            }
            break;

        case LOCKED_ON:
            handle_state_locked(is_on, EXPECTED_ON, key_info);
            break;

        case ON:
            handle_state_steady(is_on, EXPECTED_OFF, key_info);
            break;

        case TURNING_OFF:
            if (handle_state_turning_x(is_on, EXPECTED_OFF, key_info) == LOCKED_OFF) {
                changes |= _BV(i);
            }
            break;

        case LOCKED_OFF:
            handle_state_locked(is_on, EXPECTED_OFF, key_info);
            break;
        };

    }


    debouncer->state ^= changes;

    return changes;
}
