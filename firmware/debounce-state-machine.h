#pragma once
#include <stdio.h>
#include <stdint.h>
#include "keyscanner.h"

typedef struct {
    uint8_t phase;
    uint8_t ticks;
    uint8_t timer;
    uint8_t chatters;
} key_info_t;

typedef struct {
    key_info_t key_info[8];
    uint8_t state;  // debounced state
} debounce_t;


enum lifecycle_phases { OFF, TURNING_ON, LOCKED_ON, ON, TURNING_OFF, LOCKED_OFF};

typedef struct {
    uint8_t
    next_phase: 3,
                unexpected_data_phase: 3,
                expected_data:1,
                unexpected_data_is_chatter: 1;
    int8_t regular_timer;
    int8_t chattering_switch_timer;
} lifecycle_phase_t;



lifecycle_phase_t lifecycle[] = {
    {
        // OFF
        .next_phase = OFF,
        .expected_data = 0,
        .unexpected_data_phase = TURNING_ON,
        .regular_timer = 0,
        .unexpected_data_is_chatter = 0,
        .chattering_switch_timer = 0
    },
    {
        // TURNING_ON
        .next_phase = LOCKED_ON,
        .expected_data = 1,
        .unexpected_data_phase = OFF,
        .regular_timer = 1,
        .unexpected_data_is_chatter = 0,
        .chattering_switch_timer = 2
    },
    {
        // LOCKED_ON
        .next_phase = ON,
        .expected_data = 1,
        .unexpected_data_phase = LOCKED_ON,
        .regular_timer = 14,
        .unexpected_data_is_chatter = 1,
        .chattering_switch_timer = 45
    },
    {
        // ON
        .next_phase = ON,
        .expected_data = 1,
        .unexpected_data_phase = TURNING_OFF,
        .regular_timer = 5,
        .unexpected_data_is_chatter = 0,
        .chattering_switch_timer =65
    },
    {
        // TURNING_OFF
        .next_phase = LOCKED_OFF,
        .expected_data = 0,
        .unexpected_data_phase = ON,
        .regular_timer = 20,
        .unexpected_data_is_chatter = 1,
        .chattering_switch_timer =95
    },
    {
        // LOCKED_OFF
        .next_phase = OFF,
        .expected_data = 0,
        .unexpected_data_phase =  LOCKED_OFF,
        .regular_timer = 10,
        .unexpected_data_is_chatter =1,
        .chattering_switch_timer = 30
    }
};

void transition_to_phase(key_info_t *key_info, int8_t new_phase) {
    //key_info->timer=(key_info->chatters ? lifecycle[new_phase].chattering_switch_timer: lifecycle[new_phase].regular_timer );
 
 if (key_info->phase != new_phase) {
      	 key_info->phase= new_phase;
    key_info->ticks=0;
 }
}






static uint8_t debounce(uint8_t sample, debounce_t *debouncer) {
    uint8_t changes = 0;
    // Scan each pin from the bank
    for(int8_t i=0; i< COUNT_INPUT; i++) {
        key_info_t *key_info = debouncer->key_info+i;
        key_info->ticks++;

        lifecycle_phase_t current_phase = lifecycle[key_info->phase];

        if ((sample & _BV(i)) != current_phase.expected_data) {
            // if we get the 'other' value during a locked window, that's gotta be chatter
            key_info->chatters = key_info->chatters || current_phase.unexpected_data_is_chatter;
            transition_to_phase(key_info, current_phase.unexpected_data_phase);
        }

        // do not act on any input during the locked off window
        if (key_info->ticks >
           (key_info->chatters ? lifecycle[key_info->phase].chattering_switch_timer: lifecycle[key_info->phase].regular_timer )) {
            transition_to_phase(key_info, current_phase.next_phase);

            if ( key_info->phase == LOCKED_ON  || key_info->phase == LOCKED_OFF)
                changes |= _BV(i);
        }
    }

    debouncer->state ^= changes;
    return changes;
}
