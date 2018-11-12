#include "sled1735.h"
#include <util/delay.h>
#include "main.h"
#include "keyscanner.h"
#include "wire-protocol.h"
#include <stdint.h>
#include "adc.h"

#define DETECT_ADC

uint8_t red[3] = { 255, 0, 0 };
uint8_t grn[3] = { 0, 255, 0 };
uint8_t off[3] = { 0, 0, 0 };


static inline void setup(void) {
    setup_spi(); // setup sled 1735 driver chip
    keyscanner_init();

    #ifdef DETECT_ADC
    // set CC pin to input
    SET_INPUT(DDRA,0);

    // if dpf is providing power to us
    uint16_t average = 0;
    while(true)
    {
        for(int i = 0; i < 10; i ++)
            average += read_adc(ADC_CC);
        average /= 10;


        if(average > 40)
        {
            led_set_all_to(red); 
            _delay_ms(100);
        }
        else
            break;
    }
    #endif

    // ansi iso reading
    // set input
    SET_INPUT(DDRB,1);
    // pull down
    LOW(PORTB,1);
    // read it: 0 = ISO, 1 = ANSI
    ansi_iso = PINB & _BV(1);
    
    // initialise twi
    twi_init();
}

float filtered_joint = 0;
float LPF_Beta = 0.025; // 0<ß<1

// exponential filter on the hall effect input, gets a bit of noise from the
// LED switching
void read_joint() {
    int raw_joint = read_adc(ADC_HALL);
    filtered_joint = filtered_joint - (LPF_Beta * (filtered_joint - raw_joint));
    // update global - gets read over I2C by huble
    joint = (int)filtered_joint;
}

int main(void) {
    setup();

    while(1) {
        if(keyscanner_main())
            // every time the keyscan is run, check the ADC
            read_joint();
    }
    __builtin_unreachable();
}
