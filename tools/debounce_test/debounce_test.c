#include "debounce_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// For our test implementation, we only do one row of keys
// Really, we should just do one key, but this better matches
// production code
//
// All our data will be on key 0 in this row.

// Run the program with 'v' as its only commandline arg for verbose mode (where we output more data)
// Run this program with 'i' as its only commandline arg to output the input data on one line
// 'c' as the only argument will print the number of presses and nothing else. Exit code is presses-releases
debounce_t db[1];

uint8_t print_press_count = 0;
uint8_t verbose = 0;
uint8_t print_input = 0;
uint8_t scan_counter =0;
uint8_t pin_data[4096] = { };
uint8_t debounced_data[4096] = {};


#define BUFFERSIZE 8192
void get_input () {
    char buffer[BUFFERSIZE];

    if (verbose) {
        printf("Enter some keyscan data. 0 for off. 1 for on. Lines starting with # are ignored. All other data reported as an error\n");
        printf("This program expects data to come from STDIN\n");
    }

    while( fgets(buffer, BUFFERSIZE, stdin) ) { /* break with ^D or ^Z */
        for(uint8_t i = 0; (i< sizeof(buffer) && buffer[i+1] != 0)  ; i++) {
            if(buffer[i] == '#') {
                break;
            } else if(buffer[i] == '0') {
                pin_data[scan_counter++] = 0;
            } else if (buffer[i] == '1') {

                pin_data[scan_counter++] = 1;
            } else {
                if (verbose ) {
                    printf("Bad data ignored %c\n", buffer [i]);
                }
            }
        }
    }

}



int main(int argc,char *argv[]) {
    if(argc ==2 && *argv[1] == 'v')  {
        verbose = 1;
    }
    else if(argc ==2 && *argv[1] == 'i')  {
        print_input = 1;
    } else if (argc ==2 && *argv[1] =='c') {
	print_press_count =1;
    }


    get_input();

    uint8_t presses;
    uint8_t releases;
    uint8_t debounced_changes;

    memset(db, 0, sizeof(*db) * COUNT_OUTPUT);


    for (uint8_t sample = 0; sample < scan_counter; sample++) {
            debounced_changes= debounce(pin_data[sample], db);

        if (verbose) {
            printf("Sample %-3d: %d ",sample,pin_data[sample]);
            printf("-> %d", (db[0].state ));

            if (pin_data[sample] != pin_data[sample-1]) {
                printf(" input changed to %d", pin_data[sample]);
            }
        }
        if (debounced_changes) {
            if (verbose)
                printf(" output changed to %d", (db[0].state ));
            if (!(db[0].state )) {
                releases++;
            } else {
                presses++;
            }

        }

        debounced_data[sample] = ( db[0].state ) ;

        if (verbose)
            printf("\n");
    }

    if (verbose) {
        printf("Total presses: %d\nTotal releases: %d\n",presses,releases);
        printf("\n");
    }
    if (print_press_count) {
	printf("%d\n",presses);
	exit(presses-releases);
    }


    if (verbose || print_input) {
	printf("Raw input data: ");
        for(uint8_t i =0; i< scan_counter; i++) {
            printf("%d",pin_data[i]);
        }
        printf("\n");
    }

    if (verbose || !print_input || !print_press_count) {

	printf("Debounced data: ");
        for(uint8_t i =0; i< scan_counter; i++) {
            printf("%d",debounced_data[i]);
        }
        printf("\n");
    }

    exit (0);
}

