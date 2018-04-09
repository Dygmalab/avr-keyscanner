#include "debounce_test.h"
#include "../../firmware/debounce.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// For our test implementation, we only do one row of keys
// Really, we should just do one key, but this better matches 
// production code
//
// All our data will be on key 0 in this row.

debounce_t db[] = {
    {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        0
    },
};


uint8_t scan_counter =0;
uint8_t pin_data[4096] ={ };
uint8_t debounced_data[4096] ={};


#define BUFFERSIZE 8192
void get_input () {
char buffer[BUFFERSIZE];
printf("Enter some keyscan data. 0 for off. 1 for on. Lines starting with # are ignored. All other data reported as an error\n");
printf("This program expects data to come from STDIN\n");

while( fgets(buffer, BUFFERSIZE , stdin) ) /* break with ^D or ^Z */
{
  for(uint8_t i = 0; (i< sizeof(buffer) && buffer[i+1] != 0)  ; i++) {
	if(buffer[i] == '#') {
		break;
	}
	else if(buffer[i] == '0') {
		pin_data[scan_counter++] = 0;
	} else if (buffer[i] == '1') {

		pin_data[scan_counter++] = 1;
	} else { 
  	printf("Bad data ignored %c\n", buffer [i]);
	}
  }
}

}




void main(void) {
	get_input();

	uint8_t presses;
	uint8_t releases;

	uint8_t debounced_changes;
	for (uint8_t sample = 0; sample < scan_counter; sample++) {

		

		debounced_changes= debounce(pin_data[sample], db);
		printf("Sample %d: %d ",sample,pin_data[sample]);
		printf("-> %d", !(db[0].state & 1));
	printf(" - counter %3d ", db[0].counters[0]);

	if (pin_data[sample] != pin_data[sample-1]) {
		printf(" input changed to %d", pin_data[sample]);
	}
	if (debounced_changes) {
		printf(" state changed to %d" , !(db[0].state & 1));
		if (!(db[0].state & 1)) {
			releases++;
		} else {
			presses++;
		}

	}

	debounced_data[sample] = !( db[0].state & 1) ;

	printf("\n");		
	}

	printf("Total presses: %d\nTotal releases: %d\n",presses,releases);
	printf("\n");

	for(uint8_t i =0; i< scan_counter; i++) {
		printf("%d",pin_data[i]);
	}
	printf("\n");

	for(uint8_t i =0; i< scan_counter; i++) {
		printf("%d",debounced_data[i]);
	}
	printf("\n");


	exit (0);
}

