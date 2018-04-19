#pragma once
#include <stdio.h>
#include <stdint.h>
#include "keyscanner.h"

enum { OFF, TURNING_ON, LOCKED_ON, ON, TURNING_OFF, LOCKED_OFF};

typedef struct {
    uint8_t key_states[8];
    uint8_t cycles[8];
    uint8_t per_state_data[8];
    uint8_t state;  // debounced state
} debounce_t;




static uint8_t debounce(uint8_t sample, debounce_t *debouncer) {
    uint8_t changes = 0;
    // Scan each pin from the bank
    for(int8_t i=0; i< 1;i++) { //COUNT_INPUT; i++) {
        uint8_t is_on=       !! (sample & _BV(i)) ;
        debouncer->cycles[i]++;
        switch (debouncer->key_states[i] ) {
        case OFF:
            // if we get a single input sample that's "1", transition to "TURNING_ON".

            if (is_on) {
                debouncer->key_states[i]= TURNING_ON;
                debouncer->cycles[i]=0;
            }
            break;

        case TURNING_ON:
// 	if the next input sample is "1"
            if (is_on) {
// 		transition to "LOCKED_ON"
// 		mark the debounced key as "ON"
                debouncer->key_states[i]= LOCKED_ON;
                debouncer->cycles[i]=0;
                changes |= _BV(i);
            } else {
                // 	otherwise, transition to "OFF"
                debouncer->key_states[i]= OFF;
                debouncer->cycles[i]=0;
            }
            break;

        case LOCKED_ON:
            // 	do not act on any input for 45 ms
            if(debouncer->cycles[i] < 78) {
                // 	TODO: if we get any "0" samples, that implies chatter
                break;
            }
            // 	after 45ms transition to "ON"
            debouncer->key_states[i]=ON;
            debouncer->cycles[i]=0;

            break;

        case ON:
            // 	while any of the last 10ms are "1", stay ON

            if (is_on) {
                debouncer->per_state_data[i]=20;
                // 	if all of the last 10ms of samples are "0", transition to "TURNING_OFF"
            } else {
                debouncer->per_state_data[i]--;
                if ( debouncer->per_state_data[i] == 0) {
                    debouncer->key_states[i]= TURNING_OFF;
                    debouncer->cycles[i]=0;
                    debouncer->per_state_data[i]=30;
                }
		}
                break;

            case TURNING_OFF:
                // 	listen for 10ms
                // 	if any samples are "1", transition to "ON"
                if(is_on) {
                    debouncer->key_states[i]= ON;
                    debouncer->cycles[i]=0;
                }

                debouncer->per_state_data[i]--;

                if(debouncer->per_state_data[i]==0) {
                    // 	transition to "LOCKED_OFF"
                    // 		mark the debounced key as "OFF"
                    debouncer->key_states[i]= LOCKED_OFF;
                    debouncer->cycles[i]=0;
                    changes |= _BV(i);

                }
                break;

            case LOCKED_OFF:
                // 	do not act on any input for 45ms
                if(debouncer->cycles[i] < 80) {
                    // 	TODO: if we get any "1" samples, that implies chatter
                    break;
                }
                // 	after 45ms transition to "OFF"
                debouncer->key_states[i]=OFF;
                debouncer->cycles[i]=0;

                break;
            };

    }


    debouncer->state ^= changes;

    return changes;
}
