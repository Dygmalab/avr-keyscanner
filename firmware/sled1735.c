/*
look at the lora radio code that had the defines for all the registers
eg #define PICTURE_MODE_BIT 3

look at the kaleidoscope keyboard mapping code for an led mapping #define


to test:
* clock polarity and phase
* configuration reg

*/

#include <stdint.h>
#include "sled1735.h"
#include <avr/delay.h>
#include "main.h"


#define CHECK_ID
#define SETUP
//#define BREATH
//#define LED_ON
#define LED_PWM
//#define LED_FADE
//#define LOOP_DELAY
#define CONST_CURR

#define SPI_D 1

// sled1735 latches data at clock rising edge, max freq is 2.4MHz
// attiny clock is 8MHz, so can divide by 4 and run at 2MHz
//SPISettings settings(SPI_CLOCK_DIV9, MSBFIRST, SPI_MODE0);

#define SHUTDOWN_PIN 6 //shutdown when low
#define SS_PIN 7
#define DDR_SPI DDRB
#define DD_MOSI 3
#define DD_SCK 5
#define SS_PIN_2 2 // the real SS pin

uint8_t stat = 0;
volatile bool done = false;
// hold the chip in shutdown until configured


void SPI_MasterInit(void)
{
    /* Set MOSI and SCK output, all others input */
    DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK)|(1<<SS_PIN)|(1<<SHUTDOWN_PIN)|(1<<SS_PIN_2);

    HIGH(PORTB,SHUTDOWN_PIN); // shutdown pin high to enable sled
    HIGH(PORTB,SS_PIN);
    HIGH(PORTB,SS_PIN_2); // ss pin must be a high output for SPI to work correctly as master
    _delay_ms(100); // wait for chip to be ready

    /* Enable SPI, Master, set clock rate fck/16 */
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

void SPI_MasterTransmit(char cData)
{
    /* Start transmission */
    SPDR = cData;
    /* Wait for transmission complete */
    while(!(SPSR & (1<<SPIF)));
}

void setup_spi()
{
    SPI_MasterInit(); 

    #ifdef CHECK_ID
    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    // 1st 4bits is: 0xA read, 0x2 write
    // 2nd 4bits is: 0x0 frame1, 0x1 frame2, 0xB function, 0xC detect, 0xD led Vaf
    // eg 0x20 - write to frame1

    // header: read function
    SPI_MasterTransmit(0xAB);


    // check ID returns OK
    // reg 0x1B is ID
    SPI_MasterTransmit(0x1B);

    SPI_MasterTransmit(0x00);
    stat = SPDR;

    // data returned should be 0111 0010 = 0x72

    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    #endif
    #ifdef SETUP
    
    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    // header: write function
    SPI_MasterTransmit(0x2B); 
    // reg 0x00: configuration
    SPI_MasterTransmit(0x00);
    // enable picture mode bit 3 set low - what does picture mode do? Do we need it?
    SPI_MasterTransmit(0b00000000);

    HIGH(PORTB, SS_PIN);
    _delay_ms(SPI_D);
    
    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    // header: write function
    SPI_MasterTransmit(0x2B); 
    // reg 0x01: matrix type
    SPI_MasterTransmit(0x01);
    // choose matrix type 3 
    SPI_MasterTransmit(0b00010000); // 0x10

    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    // check it
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0xAB);
    SPI_MasterTransmit(0x01);
    SPI_MasterTransmit(0x00);
    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    // header: write function
    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);
    SPI_MasterTransmit(0x2B); 
    SPI_MasterTransmit(0x0A);
    // turn shutdown onto normal mode, (controlled by external pin?)
    SPI_MasterTransmit(0b00000001);
    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    #endif

    #ifdef CONST_CURR
    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);
    SPI_MasterTransmit(0x2B); 
    //reg 0x0F: constant current
    SPI_MasterTransmit(0x0F);
    SPI_MasterTransmit(0b10011001); // set to 25 : 8 + (25-1)*0.5 = 20 mA
    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);
    #endif

    #ifdef BREATH
    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    // header: write function
    SPI_MasterTransmit(0x2B); 
    // reg 0x08: breath in and out time
    SPI_MasterTransmit(0x08);
    // enable breath, continuous, 1ms extinguish time
    SPI_MasterTransmit(0b00110011);

    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);
    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    // header: write function
    SPI_MasterTransmit(0x2B); 
    // reg 0x09: breath extinguish time and auto play
    SPI_MasterTransmit(0x09);
    // enable breath, continuous, 1ms extinguish time
    SPI_MasterTransmit(0b00110111);

    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    #endif

    // turn on pwm to full
    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);
    // header: write frame 1
    SPI_MasterTransmit(0x20); 
    // reg 0x20: pwm, 8 bits per led, 128 bytes for 128 leds
    SPI_MasterTransmit(0x20); 
    // do 128 times to get all LEDS
    for(int i=0; i<128; i++)
        SPI_MasterTransmit(0xFF); // half bright

    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);
    // header: write frame 2
    SPI_MasterTransmit(0x21); 
    // reg 0x20: pwm, 8 bits per led, 128 bytes for 128 leds
    SPI_MasterTransmit(0x20); 
    // do 128 times to get all LEDS
    for(int i=0; i<128; i++)
        SPI_MasterTransmit(0x00); // half bright

    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    // all leds off
    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    // header: write frame 1
    SPI_MasterTransmit(0x20); 
    // reg 0x00: led on/off, 1 bit per led, 16 bytes for 128 leds
    SPI_MasterTransmit(0x00); 

    // write 0xFF 16 times to get all 128 LEDs in first frame turned on
    // auto increment means don't need to change start reg
    for(int i=0; i<16; i++)
        SPI_MasterTransmit(0xFF);

    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);
    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);
    // header: write frame 2
    SPI_MasterTransmit(0x21); 
    // reg 0x00: led on/off, 1 bit per led, 16 bytes for 128 leds
    SPI_MasterTransmit(0x00); 

    // write 0xFF 16 times to get all 128 LEDs in first frame turned on
    // auto increment means don't need to change start reg
    for(int i=0; i<16; i++)
        SPI_MasterTransmit(0xFF);

    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);

}
int j = 0;
void sled_test()
{
/*
    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);
    SPI_MasterTransmit(0x20); 
    SPI_MasterTransmit(4); 
    SPI_MasterTransmit(1<<2);

    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    _delay_ms(250);

    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);
    SPI_MasterTransmit(0x20); 
    SPI_MasterTransmit(4); 
    SPI_MasterTransmit(0);

    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    _delay_ms(250);
    */

    #ifdef LED_ON
    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    // header: write frame 1
    SPI_MasterTransmit(0x20); 
    // reg 0x00: led on/off, 1 bit per led, 16 bytes for 128 leds
    SPI_MasterTransmit(0x00); 

    // write 0xFF 16 times to get all 128 LEDs in first frame turned on
    // auto increment means don't need to change start reg
    for(int i=0; i<16; i++)
        SPI_MasterTransmit(0xFF);

    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);
    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);
    // header: write frame 2
    SPI_MasterTransmit(0x21); 
    // reg 0x00: led on/off, 1 bit per led, 16 bytes for 128 leds
    SPI_MasterTransmit(0x00); 

    // write 0xFF 16 times to get all 128 LEDs in first frame turned on
    // auto increment means don't need to change start reg
    for(int i=0; i<16; i++)
        SPI_MasterTransmit(0xFF);

    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);
    #endif

    #ifdef LED_PWM

    j += 1;
    if(j == 3)  
        j = 0;

    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);
    // header: write frame 1
    SPI_MasterTransmit(0x20); 
    // reg 0x20: pwm, 8 bits per led, 128 bytes for 128 leds
    SPI_MasterTransmit(0x20); 
    // do 128 times to get all LEDS
    for(int i=0; i<128; i++)
        SPI_MasterTransmit(i % 3 == j ? 0xFF : 0x00); // half bright

    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);
    // header: write frame 2
    SPI_MasterTransmit(0x21); 
    // reg 0x20: pwm, 8 bits per led, 128 bytes for 128 leds
    SPI_MasterTransmit(0x20); 
    // do 128 times to get all LEDS
    for(int i=0; i<128; i++)
        SPI_MasterTransmit(i % 3 == j ? 0xFF : 0x00); // half bright

    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    #endif

    #ifdef LED_FADE
    // try to fade in and out each LED in turn
    LOW(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    // header: write frame 1
    SPI_MasterTransmit(0x20); 
    // reg 0x20: pwm, 8 bits per led, 128 bytes for 128 leds
    SPI_MasterTransmit(0x20); 
    // do 128 times to get all LEDS
    for(int i=0; i<128; i++)
        SPI_MasterTransmit(0x00); // all off

    HIGH(PORTB,SS_PIN);
    _delay_ms(SPI_D);

    // each LED address
    for(int addr=0x20; addr<0xA0; addr++)
    {
        for(int pwm=0; pwm <=0xFF; pwm ++)
        {
            LOW(PORTB,SS_PIN);
            SPI_MasterTransmit(0x20); // write frame 1
            SPI_MasterTransmit(addr); // PWM reg of current LED
            SPI_MasterTransmit(pwm);  // write PWM val
            HIGH(PORTB,SS_PIN);
        }
        for(int pwm=0xFF; pwm >=0; pwm --)
        {
            LOW(PORTB,SS_PIN);
            SPI_MasterTransmit(0x20); 
            SPI_MasterTransmit(addr); 
            SPI_MasterTransmit(pwm);
            HIGH(PORTB,SS_PIN);
        }
    }
    #endif

    #ifdef LOOP_DELAY
    _delay_ms(500);
    #endif
}

