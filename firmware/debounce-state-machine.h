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



uint8_t transition_to_state(key_info_t *key_info, uint8_t new_state) {
    key_info->lifecycle= new_state;
    key_info->ticks=0;
    key_info->timer=(key_info->chatters ? chattering_switch_timers[new_state]: regular_timers[new_state] );
    return new_state;
}


void chatter_detected ( key_info_t *key_info) {
    key_info->chatters=1;
    key_info->timer= chattering_switch_timers[key_info->lifecycle];

}

int8_t handle_state_off (uint8_t is_on, key_info_t *key_info) {
    if (!is_on) {
        return transition_to_state(key_info, OFF);
    }

    // if we get a single input sample that's "1", transition to "TURNING_ON".
    if ( key_info->ticks > key_info->timer ) {
        return transition_to_state(key_info, TURNING_ON);
    }
    return key_info->lifecycle;
}

int8_t handle_state_on(uint8_t is_on, key_info_t *key_info) {
    if (is_on) {
        return transition_to_state(key_info, ON);
    }

    // 	if all of the last 10ms of samples are "0", transition to "TURNING_OFF"
    if ( key_info->ticks > key_info->timer ) {
        return transition_to_state(key_info,  TURNING_OFF);
    }
    return key_info->lifecycle;
}

int8_t handle_state_turning_on(uint8_t is_on, key_info_t *key_info) {
    if (!is_on)  {
        chatter_detected(key_info);
        return transition_to_state(key_info, OFF);
    }

    if ( key_info->ticks > key_info->timer ) {
        return transition_to_state(key_info, LOCKED_ON);
    }

    return key_info->lifecycle;
}


int8_t handle_state_turning_off(uint8_t is_on, key_info_t *key_info) {
    if(is_on) {
        chatter_detected(key_info);
        return transition_to_state(key_info,  ON);
    }

    if( key_info->ticks > key_info->timer ) {
        return transition_to_state(key_info,  LOCKED_OFF);
    }
    return key_info->lifecycle;
}

int8_t handle_state_locked_on(uint8_t is_on, key_info_t *key_info) {
    if (!is_on) {
        chatter_detected(key_info);
    }

    // 	do not act on any input while the key is locked on
    if(key_info->ticks > key_info->timer) {
        return transition_to_state(key_info, ON);
    }
    return key_info->lifecycle;
}


int8_t handle_state_locked_off(uint8_t is_on, key_info_t *key_info) {
    // 	do not act on any input during the locked off window
    if (is_on) {
        chatter_detected(key_info);
    }

    // 	after 45ms transition to "OFF"
    if(key_info->ticks > key_info->timer ) {
        return transition_to_state(key_info, OFF);
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
            handle_state_off(is_on, key_info);
            break;

        case TURNING_ON:
            if (handle_state_turning_on(is_on, key_info) == LOCKED_ON) {
                changes |= _BV(i);
            }
            break;

        case LOCKED_ON:
            handle_state_locked_on(is_on, key_info);
            break;

        case ON:
            handle_state_on(is_on,key_info);
            break;

        case TURNING_OFF:
            if (handle_state_turning_off(is_on, key_info) == LOCKED_OFF) {
                changes |= _BV(i);
            }
            break;

        case LOCKED_OFF:
            handle_state_locked_off(is_on, key_info);
            break;
        };

    }


    debouncer->state ^= changes;

    return changes;
}
