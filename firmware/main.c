#include "sled1735.h"
#include <util/delay.h>
#include "main.h"
#include "keyscanner.h"
#include "wire-protocol.h"
#include <stdint.h>
#include "adc.h"
#include <avr/wdt.h>

//#define DETECT_ADC

uint8_t red[3] = {255, 0, 0};
uint8_t grn[3] = {0, 255, 0};
uint8_t off[3] = {0, 0, 0};

static inline void setup(void)
{
    setup_spi(); // setup sled 1735 driver chip
    keyscanner_init();

#ifdef DETECT_ADC
    // set CC pin to input
    SET_INPUT(DDRA, 0);

    // if dpf is providing power to us
    uint16_t average = 0;
    while (true)
    {
        for (int i = 0; i < 10; i++)
            average += read_adc(ADC_CC);
        average /= 10;

        if (average > 40)
        {
            led_set_all_to(red);
            _delay_ms(100);
        }
        else
            break;
    }
#endif

    // ansi iso reading - up to v4.9 pcbs use ANSI pullup, ISO floating
    // but chip doesn't have configurable pulldowns, only pullups
    // set pin to be low output first, to drain any voltage on pin's capacitance
    SET_OUTPUT(DDRB, 1);
    LOW(PORTB, 1);

    // then set it input
    SET_INPUT(DDRB, 1);
    // leave enough time full the pullup to change the voltage on the pin
    asm("nop");
    // then read: 0 = ISO, 2 = ANSI
    ansi_iso = (PINB & _BV(1)) == 2 ? ANSI : ISO;

    // setup watchdog
    wdt_enable(WDTO_250MS);

    // initialise twi
    twi_init();
}

// median filter on the hall effect input, gets a bit of noise from the LED switching
uint16_t joint_reads[3] = {0};
void read_joint()
{
    // rotate data through array
    joint_reads[2] = joint_reads[1];
    joint_reads[1] = joint_reads[0];
    joint_reads[0] = read_adc(ADC_HALL);

    // update global - gets read over I2C by huble
    joint = middle_of_3(joint_reads[0], joint_reads[1], joint_reads[2]);
}

int main(void)
{
    setup();

    while (1)
    {
        // every time the keyscan is run, check the ADC
        if (keyscanner_main())
            read_joint();

        if (led_bank.update == 1)
            led_send_bank();
        // if(keyscanner_main())
        // every time the keyscan is run, check the ADC
        // read_joint();
        wdt_reset();
    }
    __builtin_unreachable();
}
