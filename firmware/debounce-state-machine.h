#pragma once
#include <stdio.h>
#include <stdint.h>
#include "keyscanner.h"

#define debug(x)  //printf(x); printf("\n");


#define EXPECTED_OFF 0
#define EXPECTED_ON  1


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



enum lifecycle_states { OFF, TURNING_ON, LOCKED_ON, ON, TURNING_OFF, LOCKED_OFF, UNCHANGED};

typedef struct {
    uint8_t 
	    next_state: 3,
	    unexpected_data_state: 3,
	    expected_data:1,
            unexpected_data_is_chatter: 1;
    int8_t regular_timer;
    int8_t chattering_switch_timer;
} lifecycle_state_t;



lifecycle_state_t lifecycle[] = {
    {
        .next_state = TURNING_ON,
        .expected_data = EXPECTED_ON,
        .unexpected_data_state = OFF,
        .regular_timer = 0,
        .unexpected_data_is_chatter = 0,
        .chattering_switch_timer = 0
    },
    {
        .next_state = LOCKED_ON,
        .expected_data = EXPECTED_ON,
        .unexpected_data_state = OFF,
        .regular_timer = 1,
        .unexpected_data_is_chatter = 1,
        .chattering_switch_timer = 2
    },
    {
        .next_state = ON,
        .expected_data = EXPECTED_ON,
        .unexpected_data_state = UNCHANGED,
        .regular_timer = 7,
        .unexpected_data_is_chatter = 1,
        .chattering_switch_timer = 35
    },
    {
        .next_state = TURNING_OFF,
        .expected_data = EXPECTED_OFF,
        .unexpected_data_state = ON,
        .regular_timer = 10,
        .unexpected_data_is_chatter = 0,
        .chattering_switch_timer = 65
    },
    {
        .next_state = LOCKED_OFF,
        .expected_data = EXPECTED_OFF,
        .unexpected_data_state = ON,
        .regular_timer = 6,
        .unexpected_data_is_chatter = 1,
        .chattering_switch_timer = 27
    },
    {
        .next_state = OFF,
        .expected_data = EXPECTED_OFF,
        .unexpected_data_state =  UNCHANGED,
        .regular_timer = 30,
        .unexpected_data_is_chatter =1,
        .chattering_switch_timer = 50
    }
};

uint8_t transition_to_state(key_info_t *key_info, int8_t new_state) {
    if (new_state == UNCHANGED) {
        return key_info->lifecycle;
    }
    key_info->lifecycle= new_state;
    key_info->ticks=0;
    key_info->timer=(key_info->chatters ? lifecycle[new_state].chattering_switch_timer: lifecycle[new_state].regular_timer );
    return new_state;
}






static uint8_t debounce(uint8_t sample, debounce_t *debouncer) {
    uint8_t changes = 0;
    // Scan each pin from the bank
    for(int8_t i=0; i< COUNT_INPUT; i++) {
        uint8_t is_on=  !! (sample & _BV(i)) ;
        key_info_t *key_info	= debouncer->key_info+i;

        key_info->ticks++;

        uint8_t old_lifecycle = key_info->lifecycle;


        // if we get the 'other' value during a locked window, that's gotta be chatter
        if (is_on != lifecycle[key_info->lifecycle].expected_data) {
            if (lifecycle[key_info->lifecycle].unexpected_data_is_chatter) {
            	key_info->chatters=1;
            }
            transition_to_state(key_info, lifecycle[key_info->lifecycle].unexpected_data_state);
        }

        // 	do not act on any input during the locked off window
        if (key_info->ticks <= key_info->timer) {
		continue;
	}


	transition_to_state(key_info, lifecycle[key_info->lifecycle].next_state);


        if (( old_lifecycle == TURNING_ON && key_info->lifecycle == LOCKED_ON)  ||
                ( old_lifecycle == TURNING_OFF && key_info->lifecycle == LOCKED_OFF) ) {


            changes |= _BV(i);
        }

    }


    debouncer->state ^= changes;

    return changes;
}
