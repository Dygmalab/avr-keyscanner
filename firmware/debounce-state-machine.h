#pragma once
#include <stdio.h>
#include <stdint.h>
#include "keyscanner.h"

typedef struct {
    uint8_t phase;
    uint8_t ticks;
    uint8_t chatters;
} key_info_t;

typedef struct {
    key_info_t key_info[8];
    uint8_t state;  // debounced state
} debounce_t;


enum lifecycle_phases {
    OFF, TURNING_ON, LOCKED_ON, ON, TURNING_OFF, LOCKED_OFF,
    NOISY_SWITCH_OFF, NOISY_SWITCH_TURNING_ON, NOISY_SWITCH_LOCKED_ON, NOISY_SWITCH_ON, NOISY_SWITCH_TURNING_OFF, NOISY_SWITCH_LOCKED_OFF


};

typedef struct {
    uint8_t
    next_phase: 4,
                unexpected_data_phase: 4;
    uint8_t
    expected_data:1,
                  change_output_on_expected_transition:1;
    uint8_t timer;
} lifecycle_phase_t;



lifecycle_phase_t lifecycle[] = {
    {
        // OFF -- during this phase, any 'off' value means that we should keep this key pressed
        // A single 'on' value means that we should start checking to see if it's really a key press
        //
        // IF we get an 'on' value, change the phase to 'TURNING_ON' to make sure it's not just
        // chatter
        //
        // Our timers are set to 0, but that doesn't matter because in the event that we overflow the timer
        // we just go back to the 'OFF' phase

        .next_phase = OFF,
        .expected_data = 0,
        .unexpected_data_phase = TURNING_ON,
        .timer = 0,
    },
    {
        // TURNING_ON-- during this phase, we believe that we've detected
        // a switch being turned on. We're now checking to see if it's
        // reading consistently as 'on' or if it was just a spurious "on" signal
        // as might happen if we saw key chatter
        //
        // If it was a spurious disconnection, mark the switch as noisy and go back to phase OFF
        //
        // If we get through the timer with no "off" signals, proceed to phase LOCKED_ON

        .next_phase = LOCKED_ON,
        .expected_data = 1,
        .unexpected_data_phase = OFF,
        .timer = 1,
    },
    {
        // LOCKED_ON -- during this phase, the key is on, no matter what value we read from the input
        // pin.
        //
        // If we see any 'off' signals, that indicates a short read or chatter.
        // In the event of unexpected data, stay in the LOCKED_ON phase, but don't reset the timer.

        .next_phase = ON,
        .expected_data = 1,
        .unexpected_data_phase = NOISY_SWITCH_LOCKED_ON,
        .change_output_on_expected_transition = 1,
        .timer = 14,
    },
    {
        // ON -- during this phase, any 'on' value means that we should keep this key pressed
        // A single 'off' value means that we should start checking to see if it's really a key release
        //
        // IF we get an 'off' value, change the phase to 'TURNING_OFF' to make sure it's not just
        // chatter
        //
        // Our timers are set to 0, but that doesn't matter because in the event that we overflow the timer
        // we just go back to the 'ON' phase
        .next_phase = ON,
        .expected_data = 1,
        .unexpected_data_phase = TURNING_OFF,
        .timer = 0,
    },
    {
        // TURNING_OFF -- during this phase, we believe that we've detected
        // a switch being turned off. We're now checking to see if it's
        // reading consistently as 'off' or if it was just a spurious "off" signal
        // as might happen if we saw key chatter
        //
        // If it was a spurious connection, mark the switch as noisy and go back to phase ON
        //
        // If we get through the timer with no "on" signals, proceed to phase LOCKED_OFF
        .next_phase = LOCKED_OFF,
        .expected_data = 0,
        .unexpected_data_phase = NOISY_SWITCH_ON,
        .timer = 15,  // release latency
    },
    {
        // LOCKED_OFF -- during this phase, the key is off, no matter what value we read from the input
        // pin.
        //
        // If we see any 'on' signals, that indicates a short read or chatter.
        // In the event of unexpected data, stay in the LOCKED_OFF phase, but don't reset the timer.
        .next_phase = OFF,
        .expected_data = 0,
        .unexpected_data_phase =  NOISY_SWITCH_LOCKED_OFF,
        .change_output_on_expected_transition = 1,
        .timer = 10,
    },
    {
        // NOISY_SWITCH_OFF -- during this phase, any 'off' value means that we should keep this key pressed
        // A single 'on' value means that we should start checking to see if it's really a key press
        //
        // IF we get an 'on' value, change the phase to 'NOISY_SWITCH_TURNING_ON' to make sure it's not just
        // chatter
        //
        // Our timers are set to 0, but that doesn't matter because in the event that we overflow the timer
        // we just go back to the 'NOISY_SWITCH_OFF' phase

        .next_phase = NOISY_SWITCH_OFF,
        .expected_data = 0,
        .unexpected_data_phase = NOISY_SWITCH_TURNING_ON,
        .timer = 0
    },
    {
        // NOISY_SWITCH_TURNING_ON-- during this phase, we believe that we've detected
        // a switch being turned on. We're now checking to see if it's
        // reading consistently as 'on' or if it was just a spurious "on" signal
        // as might happen if we saw key chatter
        //
        // If it was a spurious disconnection, mark the switch as noisy and go back to phase NOISY_SWITCH_OFF
        //
        // If we get through the timer with no "off" signals, proceed to phase NOISY_SWITCH_LOCKED_ON

        .next_phase = NOISY_SWITCH_LOCKED_ON,
        .expected_data = 1,
        .unexpected_data_phase = NOISY_SWITCH_OFF,
        .timer = 2
    },
    {
        // NOISY_SWITCH_LOCKED_ON -- during this phase, the key is on, no matter what value we read from the input
        // pin.
        //
        // If we see any 'off' signals, that indicates a short read or chatter.
        // In the event of unexpected data, stay in the NOISY_SWITCH_LOCKED_ON phase, but don't reset the timer.

        .next_phase = NOISY_SWITCH_ON,
        .expected_data = 1,
        .unexpected_data_phase = NOISY_SWITCH_LOCKED_ON,
        .change_output_on_expected_transition = 1,
        .timer = 45
    },
    {
        // NOISY_SWITCH_ON -- during this phase, any 'on' value means that we should keep this key pressed
        // A single 'off' value means that we should start checking to see if it's really a key release
        //
        // IF we get an 'off' value, change the phase to 'NOISY_SWITCH_TURNING_OFF' to make sure it's not just
        // chatter
        //
        // Our timers are set to 0, but that doesn't matter because in the event that we overflow the timer
        // we just go back to the 'ON' phase
        .next_phase = NOISY_SWITCH_ON,
        .expected_data = 1,
        .unexpected_data_phase = NOISY_SWITCH_TURNING_OFF,
        .timer =0
    },
    {
        // NOISY_SWITCH_TURNING_OFF -- during this phase, we believe that we've detected
        // a switch being turned off. We're now checking to see if it's
        // reading consistently as 'off' or if it was just a spurious "off" signal
        // as might happen if we saw key chatter
        //
        // If it was a spurious connection, mark the switch as noisy and go back to phase ON
        //
        // If we get through the timer with no "on" signals, proceed to phase LOCKED_OFF
        .next_phase = NOISY_SWITCH_LOCKED_OFF,
        .expected_data = 0,
        .unexpected_data_phase = NOISY_SWITCH_ON,
        .timer = 92  // release latency
    },
    {
        // NOISY_SWITCH_LOCKED_OFF -- during this phase, the key is off, no matter what value we read from the input
        // pin.
        //
        // If we see any 'on' signals, that indicates a short read or chatter.
        // In the event of unexpected data, stay in the LOCKED_OFF phase, but don't reset the timer.
        .next_phase = NOISY_SWITCH_OFF,
        .expected_data = 0,
        .unexpected_data_phase =  NOISY_SWITCH_LOCKED_OFF,
        .change_output_on_expected_transition = 1,
        .timer = 30
    }
};

static uint8_t debounce(uint8_t sample, debounce_t *debouncer) {
    uint8_t changes = 0;
    // Scan each pin from the bank
    for(int8_t i=0; i< COUNT_INPUT; i++) {
        key_info_t *key_info = debouncer->key_info+i;
        key_info->ticks++;

        lifecycle_phase_t current_phase = lifecycle[key_info->phase];

        if ((sample & _BV(i)) != current_phase.expected_data) {
            // if we get the 'other' value during a locked window, that's gotta be chatter
            if (key_info->phase != current_phase.unexpected_data_phase) {
                key_info->phase = current_phase.unexpected_data_phase;
                key_info->ticks=0;
            }
        }

        // do not act on any input during the locked off window
        if (key_info->ticks > lifecycle[key_info->phase].timer ) {
            if (key_info->phase!= current_phase.next_phase) {
                key_info->ticks=0;
	    }
            key_info->phase= current_phase.next_phase;

            if (lifecycle[key_info->phase].change_output_on_expected_transition ) {
                changes |= _BV(i);

            }
        }
    }

    debouncer->state ^= changes;
    return changes;
}
