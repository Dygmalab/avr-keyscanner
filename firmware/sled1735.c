/*
easily address all leds:

     led_buffer.each[x][y].rgb = {0, 255, 5};

easily write them all to SPI:
    SPDR = led_buffer.bank[0].start_addr
    SPDR = led_buffer.bank[0].whole[index++];

easily receive values from I2C buf and copy to led_buffer
    memcpy((uint8_t *)led_buffer.bank[bank], buf, LED_BANK_SIZE);
    
* data comes in RGB, RGB .... RGB blocks of 8 x 3 bytes = 24 bytes
* store data in natural matrix format
* spi output uses a LUT to put the data out in the correct order

*/

#include <stdint.h>
#include "sled1735.h"
#include <string.h>
#include <util/delay.h>
#include "main.h"
#include "map.h"

#define LED_DATA_SIZE 3
#define NUM_LEDS_PER_BANK 8

/* defs for the 8x7 test board
#define COLS 8
#define ROWS 7
#define LED_DATA_SIZE 3
#define NUM_LEDS_PER_BANK 8
#define NUM_LEDS COLS * ROWS
*/

#define KEYS 30
#define LP_KEYS 2
#define PALM 14
#define UNDERGLOW 13
#define NUM_LEDS 60 
#define NUM_LED_BANKS 8
#define LED_BANK_SIZE (LED_DATA_SIZE*NUM_LEDS_PER_BANK)

#define FRAME_SIZE 128
#define NUM_FRAMES 2

typedef struct {
    uint8_t g;
    uint8_t r;
    uint8_t b;
} led_t;

typedef union {
    uint8_t whole[NUM_LEDS * LED_DATA_SIZE];
    led_t weach[NUM_LEDS];
    uint8_t bank[NUM_LED_BANKS][LED_BANK_SIZE];
} led_buffer_t ;

#define LED1 0, 0, 0
#define LED4 LED1, LED1, LED1, LED1
#define LED8 LED4, LED4
#define LED16 LED8, LED8
#define LED32 LED16, LED16
#define LED64 LED32, LED32
#define LED128 LED64, LED64

led_buffer_t led_buffer = { LED64 }; // 59 RGBs



#define CHECK_ID
#define SETUP
//#define BREATH
//#define LED_ON
//#define LED_PWM
//#define LED_FADE
//#define LOOP_DELAY
//#define DELAY_TIME 200
#define CONST_CURR
#define INIT_PWM 0xFF
//#define BLINK_TEST
//#define MAP_TEST
#define INT_TEST


#define SHUTDOWN_PIN 6 //shutdown when low
#define SS_PIN 2
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
    _delay_ms(1); // wait for chip to be ready

    // sled1735 latches data at clock rising edge, max freq is 2.4MHz
    // attiny clock is 8MHz, so can divide by 4 and run at 2MHz
    /* Enable SPI, Master, set clock rate fck/16 */
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1);
    SPSR ^= _BV(SPI2X);
}

void SPI_MasterTransmit(char cData)
{
    /* Start transmission */
    SPDR = cData;
    /* Wait for transmission complete */
    while(!(SPSR & (1<<SPIF)));
}

#define R 0
#define G 50
#define B 200
void led_update_bank(uint8_t *buf, const uint8_t bank) {

    memcpy(&led_buffer.bank[bank], buf, LED_BANK_SIZE);
}

void led_set_one_to(uint8_t led, uint8_t *buf) {
    //overflow possible here
    memcpy(&led_buffer.weach[led], buf, LED_DATA_SIZE);
}

void led_set_all_to( uint8_t *buf) {
    for(uint8_t led=0; led <NUM_LEDS; led++) {
        memcpy(&led_buffer.weach[led], buf, LED_DATA_SIZE);
        }
}

//        memcpy((uint8_t *)led_buffer.each[led], buf, LED_DATA_SIZE);

void setup_spi()
{
    SPI_MasterInit(); 

    #ifdef CHECK_ID
    LOW(PORTB,SS_PIN);
    

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
    

    #endif
    #ifdef SETUP
    
    LOW(PORTB,SS_PIN);
    

    // header: write function
    SPI_MasterTransmit(0x2B); 
    // reg 0x00: configuration
    SPI_MasterTransmit(0x00);
    // enable picture mode bit 3 set low - what does picture mode do? Do we need it?
    SPI_MasterTransmit(0b00000000);

    HIGH(PORTB, SS_PIN);
    
    
    LOW(PORTB,SS_PIN);
    

    // header: write function
    SPI_MasterTransmit(0x2B); 
    // reg 0x01: matrix type
    SPI_MasterTransmit(0x01);
    // choose matrix type 3 
    SPI_MasterTransmit(0b00010000); // 0x10

    HIGH(PORTB,SS_PIN);
    

    // check it
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0xAB);
    SPI_MasterTransmit(0x01);
    SPI_MasterTransmit(0x00);
    HIGH(PORTB,SS_PIN);
    

    // header: write function
    LOW(PORTB,SS_PIN);
    
    SPI_MasterTransmit(0x2B); 
    SPI_MasterTransmit(0x0A);
    // turn shutdown onto normal mode, (controlled by external pin?)
    SPI_MasterTransmit(0b00000001);
    HIGH(PORTB,SS_PIN);
    

    #endif

    #ifdef CONST_CURR
    LOW(PORTB,SS_PIN);
    
    SPI_MasterTransmit(0x2B); 
    //reg 0x0F: constant current
    SPI_MasterTransmit(0x0F);
    SPI_MasterTransmit(0b10111111); // set to 25 : 8 + (25-1)*0.5 = 20 mA
    HIGH(PORTB,SS_PIN);
    
    #endif

    #ifdef BREATH
    LOW(PORTB,SS_PIN);
    

    // header: write function
    SPI_MasterTransmit(0x2B); 
    // reg 0x08: breath in and out time
    SPI_MasterTransmit(0x08);
    // enable breath, continuous, 1ms extinguish time
    SPI_MasterTransmit(0b01000100);

    HIGH(PORTB,SS_PIN);
    
    LOW(PORTB,SS_PIN);
    

    // header: write function
    SPI_MasterTransmit(0x2B); 
    // reg 0x09: breath extinguish time and auto play
    SPI_MasterTransmit(0x09);
    // enable breath, continuous, 1ms extinguish time
    SPI_MasterTransmit(0b00110111);

    HIGH(PORTB,SS_PIN);
    

    #endif

    // all leds on
    LOW(PORTB,SS_PIN);
    

    // header: write frame 1
    SPI_MasterTransmit(0x20); 
    // reg 0x00: led on/off, 1 bit per led, 16 bytes for 128 leds
    SPI_MasterTransmit(0x00); 

    // write 0xFF 16 times to get all 128 LEDs in first frame turned on
    // auto increment means don't need to change start reg
    for(int i=0; i<16; i++)
        SPI_MasterTransmit(0xFF);

    HIGH(PORTB,SS_PIN);
    

    LOW(PORTB,SS_PIN);
    
    // header: write frame 2
    SPI_MasterTransmit(0x21); 
    // reg 0x00: led on/off, 1 bit per led, 16 bytes for 128 leds
    SPI_MasterTransmit(0x00); 

    // write 0xFF 16 times to get all 128 LEDs in first frame turned on
    // auto increment means don't need to change start reg
    for(int i=0; i<16; i++)
        SPI_MasterTransmit(0xFF);

    HIGH(PORTB,SS_PIN);
   

    #ifdef INIT_PWM
    LOW(PORTB,SS_PIN);
    
    // header: write frame 2
    SPI_MasterTransmit(0x20); 
    // reg 0x00: led on/off, 1 bit per led, 16 bytes for 128 leds
    SPI_MasterTransmit(0x20); 

    // write 0xFF 16 times to get all 128 LEDs in first frame turned on
    // auto increment means don't need to change start reg
    for(int i=0; i<128; i++)
        SPI_MasterTransmit(INIT_PWM);

    HIGH(PORTB,SS_PIN);

    LOW(PORTB,SS_PIN);
    
    // header: write frame 2
    SPI_MasterTransmit(0x21); 
    // reg 0x00: led on/off, 1 bit per led, 16 bytes for 128 leds
    SPI_MasterTransmit(0x20); 

    // write 0xFF 16 times to get all 128 LEDs in first frame turned on
    // auto increment means don't need to change start reg
    for(int i=0; i<128; i++)
        SPI_MasterTransmit(INIT_PWM);

    HIGH(PORTB,SS_PIN);
    #endif
    

    #ifdef INT_TEST
    SPCR |= (1<<SPIE);
    sei();
    #endif
}
int j = 0;
int i = 0;
const int num_leds = 22;
const int order[] = { 0, 1, 2, 3, 4, 5, 6 ,7, 8, 9, 10, 11, 12, 13, 14, 15, 22, 36, 42, 49, 50, 51 };
uint8_t r, g, b = 0;
//////////////////////////////////////////////////////////////////////////////////////////////////
void sled_test()
{

    #ifdef INT_TEST_PULSE
    // pulse through
    for(int i = num_leds - 1; i > 0; i --)
        led_buffer.weach[order[i]] = led_buffer.weach[order[i-1]];
    j ++;
    if( j % 3 == 0)
    {
        led_buffer.weach[order[0]].r = 255;
        led_buffer.weach[order[0]].g = 0;
        led_buffer.weach[order[0]].b = 0;
    }
    else if(j %3 == 1)
    {
        led_buffer.weach[order[0]].r = 0;
        led_buffer.weach[order[0]].g = 255;
        led_buffer.weach[order[0]].b = 0;
    }
    else
    {
        led_buffer.weach[order[0]].r = 0;
        led_buffer.weach[order[0]].g = 0;
        led_buffer.weach[order[0]].b = 255;
    }

    #endif

    #ifdef MAP_TEST
    j++;
    led_buffer.weach[order[0]].r += 15;
    led_buffer.weach[order[0]].g -= 3;
    led_buffer.weach[order[0]].b += 7;

    // access by frame
     
    uint8_t frames[NUM_FRAMES][FRAME_SIZE] = FRAME_MAP( led_buffer.whole );

    for( int f = 0; f < NUM_FRAMES; f++ )
    {
        LOW(PORTB,SS_PIN);
        

        // header: write frame 1
        SPI_MasterTransmit(0x20 + f); 
        // pwm reg
        SPI_MasterTransmit(0x20);  

        for(int i = 0; i < FRAME_SIZE; i ++)
        {
            SPI_MasterTransmit(frames[f][i]);  
        }
        HIGH(PORTB,SS_PIN);
        
    }

    // pulse through
    for(int i = num_leds - 1; i > 0; i --)
        led_buffer.weach[order[i]] = led_buffer.weach[order[i-1]];

    #endif

    #ifdef BLINK_TEST
    for(uint8_t i = 0; i < 8; i ++)
    {
        LOW(PORTB,SS_PIN);
        

        // header: write frame 2
        SPI_MasterTransmit(0x20); 
        // reg 0x00: led on/off, 1 bit per led, 16 bytes for 128 leds
        SPI_MasterTransmit(0x20 + i); 

        SPI_MasterTransmit(0xFF);

        HIGH(PORTB,SS_PIN);
        

        _delay_ms(200);
        LOW(PORTB,SS_PIN);
        

        // header: write frame 2
        SPI_MasterTransmit(0x20); 
        // reg 0x00: led on/off, 1 bit per led, 16 bytes for 128 leds
        SPI_MasterTransmit(0x20 + i); 

        SPI_MasterTransmit(0x00);

        HIGH(PORTB,SS_PIN);
        _delay_ms(200);

    }
    #endif

    #ifdef LED_ON
    LOW(PORTB,SS_PIN);
    

    // header: write frame 1
    SPI_MasterTransmit(0x20); 
    // reg 0x00: led on/off, 1 bit per led, 16 bytes for 128 leds
    SPI_MasterTransmit(0x00); 

    // write 0xFF 16 times to get all 128 LEDs in first frame turned on
    // auto increment means don't need to change start reg
    for(int i=0; i<16; i++)
        SPI_MasterTransmit(0xFF);

    HIGH(PORTB,SS_PIN);
    
    LOW(PORTB,SS_PIN);
    
    // header: write frame 2
    SPI_MasterTransmit(0x21); 
    // reg 0x00: led on/off, 1 bit per led, 16 bytes for 128 leds
    SPI_MasterTransmit(0x00); 

    // write 0xFF 16 times to get all 128 LEDs in first frame turned on
    // auto increment means don't need to change start reg
    for(int i=0; i<16; i++)
        SPI_MasterTransmit(0xFF);

    HIGH(PORTB,SS_PIN);
    
    #endif

    #ifdef LED_PWM

    static int amount = 0;
    for(int i=0; i<64; i++)
    {
    LOW(PORTB,SS_PIN);
    
    // header: write frame 1
    SPI_MasterTransmit(0x20); 
    // reg 0x20: pwm, 8 bits per led, 128 bytes for 128 leds
    SPI_MasterTransmit(0x20 + i); 
    // do 128 times to get all LEDS
    SPI_MasterTransmit(amount); 
    HIGH(PORTB,SS_PIN);
    
    }
    /*
    for(int i=0; i<128; i++)
    {
    LOW(PORTB,SS_PIN);
    
    // header: write frame 2
    SPI_MasterTransmit(0x21); 
    // reg 0x20: pwm, 8 bits per led, 128 bytes for 128 leds
    SPI_MasterTransmit(0x20 + i); 
    // do 128 times to get all LEDS
    SPI_MasterTransmit(amount); 
    HIGH(PORTB,SS_PIN);
    
    }
    */

    if(amount == 0)
        amount =  0x0F;
    else
        amount = 0x00;

/*
    LOW(PORTB,SS_PIN);
    
    // header: write frame 2
    SPI_MasterTransmit(0x21); 
    // reg 0x20: pwm, 8 bits per led, 128 bytes for 128 leds
    SPI_MasterTransmit(0x20); 
    // do 128 times to get all LEDS
    for(int i=0; i<128; i++)
        SPI_MasterTransmit(i % 3 == j ? 0xFF : 0x00); // half bright

    HIGH(PORTB,SS_PIN);
    
    */

    #endif

    #ifdef LED_FADE
    // try to fade in and out each LED in turn
    LOW(PORTB,SS_PIN);
    

    // header: write frame 1
    SPI_MasterTransmit(0x20); 
    // reg 0x20: pwm, 8 bits per led, 128 bytes for 128 leds
    SPI_MasterTransmit(0x20); 
    // do 128 times to get all LEDS
    for(int i=0; i<128; i++)
        SPI_MasterTransmit(0x00); // all off

    HIGH(PORTB,SS_PIN);
    

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
    _delay_ms(DELAY_TIME);
    #endif
}

uint8_t volatile led_num = 0;
uint8_t volatile led_frame = 0;
uint8_t volatile led_pos = 0;
static volatile enum { BANK, REG, DATA, END } led_state;
/* Each time a byte finishes transmitting, queue the next one */
ISR(SPI_STC_vect) {
    switch(led_state) {
    case BANK:
        LOW(PORTB,SS_PIN);
        asm("nop");
        SPDR = 0x20 + led_frame;  // select the correct frame
        led_state = REG;
        break;
    case REG:
        // pwm reg
        SPDR = 0x20;  // pwm data starts at 0x20
        led_state = DATA;
        break;
    case DATA:
    {
        led_pos = pgm_read_byte_near(&led_LUT[led_frame][led_num]);
        if(led_pos == 0xFF) // if not a valid led
            SPDR = 0;
        else
            SPDR = led_buffer.whole[led_pos];
        led_num ++;
        if( led_num == FRAME_SIZE )
        {
            led_state = END;
            led_num = 0;
            
            led_frame ++;
            if(led_frame == NUM_FRAMES)
                led_frame = 0;
        }
        break;

    }
    case END:
        SPDR = 0;
        HIGH(PORTB,SS_PIN);
        led_state = BANK;
        break;
        
    default:
    led_state = BANK;
    }
}
