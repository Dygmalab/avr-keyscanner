#pragma once
#include <stdio.h>
#include <stdint.h>
#include "keyscanner.h"

#define TURNING_ON_CHATTER_WINDOW 2 
#define TURNING_OFF_CHATTER_WINDOW 3
#define KEY_ON_CHATTER_WINDOW  5
#define LOCKED_ON_PERIOD 11
#define LOCKED_OFF_PERIOD 34

# define BAD_SWITCH_TURNING_ON_CHATTER_WINDOW 2
# define BAD_SWITCH_TURNING_OFF_CHATTER_WINDOW 35
# define BAD_SWITCH_KEY_ON_CHATTER_WINDOW  10
# define BAD_SWITCH_LOCKED_ON_PERIOD 22
# define BAD_SWITCH_LOCKED_OFF_PERIOD 50



#define debug(x)  //printf(x); printf("\n");


enum { OFF, TURNING_ON, LOCKED_ON, ON, TURNING_OFF, LOCKED_OFF};

typedef struct {
    uint8_t key_states[8];
    uint8_t cycles[8];
    uint8_t per_state_data[8];
    uint8_t key_chatters[8];
    uint8_t state;  // debounced state
} debounce_t;




static uint8_t debounce(uint8_t sample, debounce_t *debouncer) {
    uint8_t changes = 0;
    // Scan each pin from the bank
    for(int8_t i=0; i< 1; i++) { //COUNT_INPUT; i++) {
        uint8_t is_on=       !! (sample & _BV(i)) ;
        debouncer->cycles[i]++;
        switch (debouncer->key_states[i] ) {
        case OFF:
            // if we get a single input sample that's "1", transition to "TURNING_ON".

            if (is_on) {
                debouncer->key_states[i]= TURNING_ON;
                debouncer->cycles[i]=0;
                debouncer->per_state_data[i]=(debouncer->key_chatters[i] ? BAD_SWITCH_TURNING_ON_CHATTER_WINDOW : TURNING_ON_CHATTER_WINDOW);
                debug("OFF to TURNING_ON");
            }
            break;

        case TURNING_ON:
	   
	    if (!is_on)  {
                // 	otherwise, transition to "OFF"
                debouncer->key_states[i]= OFF;
                debug("TURNING_ON to OFF");
                debouncer->cycles[i]=0;
                debouncer->key_chatters[i]=1;
                debug("Chatter detected");
            } 
	    
            debouncer->per_state_data[i]--;

            if(debouncer->per_state_data[i]==0) {
                debouncer->key_states[i]= LOCKED_ON;
                debug("TURNING_ON to LOCKED_ON");
                debouncer->cycles[i]=0;
// 		mark the debounced key as "ON"
                changes |= _BV(i);
            }
            break;

        case LOCKED_ON:
            // 	do not act on any input while the key is locked on
            if(debouncer->cycles[i] < (debouncer->key_chatters[i] ?  BAD_SWITCH_LOCKED_ON_PERIOD : LOCKED_ON_PERIOD ) ){
		//printf("Locked on for %d of %d\n", debouncer->cycles[i], (debouncer->key_chatters[i] ?  BAD_SWITCH_LOCKED_ON_PERIOD : LOCKED_ON_PERIOD));
                if (!is_on) {
                    debouncer->key_chatters[i]=1;
                    debug("Chatter detected");
	//	    debouncer->cycles[i] = 0;
                }
                break;
            }

            debouncer->key_states[i]=ON;
            debug("LOCKED_ON to ON");
            debouncer->cycles[i]=0;

            break;

        case ON:
            if (is_on) {
                debouncer->per_state_data[i]=(debouncer->key_chatters[i] ? BAD_SWITCH_KEY_ON_CHATTER_WINDOW : KEY_ON_CHATTER_WINDOW);
		
            } 
                // 	if all of the last 10ms of samples are "0", transition to "TURNING_OFF"
	    else {
                debouncer->per_state_data[i]--;
                if ( debouncer->per_state_data[i] == 0) {
                    debouncer->key_states[i]= TURNING_OFF;
                    debug("ON to TURNING_OFF");
                    debouncer->cycles[i]=0;
                    debouncer->per_state_data[i]=(debouncer->key_chatters[i] ? BAD_SWITCH_TURNING_OFF_CHATTER_WINDOW : TURNING_OFF_CHATTER_WINDOW );
		    
                }
            }
            break;

        case TURNING_OFF:
            if(is_on) {
                debouncer->key_states[i]= ON;
                debug("TURNING OFF to ON");
                debouncer->cycles[i]=0;
                debouncer->key_chatters[i]=1;
                debug("Chatter detected");
            }

            debouncer->per_state_data[i]--;

            if(debouncer->per_state_data[i]==0) {
                // 		mark the debounced key as "OFF"
                debouncer->key_states[i]= LOCKED_OFF;
                debug("TURNING_OFF to LOCKED_OFF - this is when we toggle the bit");
                debouncer->cycles[i]=0;
                changes |= _BV(i);

            }
            break;

        case LOCKED_OFF:
            // 	do not act on any input during the locked off window
            if(debouncer->cycles[i] < (debouncer->key_chatters[i] ? BAD_SWITCH_LOCKED_OFF_PERIOD : LOCKED_OFF_PERIOD )) {
                // 	TODO: if we get any "1" samples, that implies chatter
                if (is_on) {
                    debouncer->key_chatters[i]=1;
                    debug("Chatter detected");
		    debouncer->cycles[i] = 0;
                }
                break;
            }
            // 	after 45ms transition to "OFF"
            debouncer->key_states[i]=OFF;
            debug("Transitioning to OFF");
            debouncer->cycles[i]=0;

            break;
        };

    }


    debouncer->state ^= changes;

    return changes;
}
