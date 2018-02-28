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

#define NUM_LEDS 72 
#define NUM_LED_BANKS 9
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

led_buffer_t led_buffer = { LED64, LED8 };

#define CHECK_ID
#define SETUP
#define CONST_CURR
#define SPI_INTS
#define SELF_TEST
#define VAF
#define INIT_PWM 0x00
//#define BREATHE

#ifdef VAF
// k2 and i4 are lit red when off
// k2 is leds c4-k, c5-k, c6-k
const uint8_t tabLED_Type3Vaf[64] = { //Reference SLED1735 Datasheet Type3 Circuit Map
  //Frame  1
  //DCBA  HGFE  LKJI  PONM
    0x50, 0x55, 0x55, 0x55, //C1-A ~ C1-P
    0x00, 0x00, 0x00, 0x00, //C2-A ~ C2-P ,
    0x00, 0x00, 0x00, 0x00, //C3-A ~ C3-P  

    0x15, 0x54, 0x55, 0x55, //C4-A ~ C4-P 
    0x00, 0x00, 0x00, 0x00, //C5-A ~ C5-P  
    0x00, 0x00, 0x00, 0x00, //C6-A ~ C6-P 

    0x55, 0x05, 0x55, 0x55, //C7-A ~ C7-P  
    0x00, 0x00, 0x00, 0x00, //C8-A ~ C8-P
    //Frame 2
    0x00, 0x00, 0x00, 0x00, //C9-A ~ C9-P 

    0x55, 0x55, 0x41, 0x55, //C10-A ~ C10-P 
    0x00, 0x00, 0x00, 0x00, //C11-A ~ C11-P  
    0x00, 0x00, 0x00, 0x00, //C12-A ~ C12-P 

    0x55, 0x55, 0x55, 0x50, //C13-A ~ C13-P  
    0x00, 0x00, 0x00, 0x00, //C14-A ~ C14-P 
    0x00, 0x00, 0x00, 0x00, //C15-A ~ C15-P 
    0x00, 0x00, 0x00, 0x00, //C16-A ~ C16-P 
};
#endif

#define SHUTDOWN_PIN 6 //shutdown when low
#define SS_PIN 7
#define DDR_SPI DDRB
#define DD_MOSI 3
#define DD_SCK 5
#define SS_PIN_2 2 // the real SS pin


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


void setup_spi()
{
    SPI_MasterInit(); 

    // 1st 4bits is: 0xA read, 0x2 write
    // 2nd 4bits is: 0x0 frame1, 0x1 frame2, 0xB function, 0xC detect, 0xD led Vaf
    // eg 0x20 - write to frame1
    

    // some stuff has to be done in shutdown
//        SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, SW_SHUT_DOWN_REG, mskSW_SHUT_DOWN_MODE);           
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0x2B);
    SPI_MasterTransmit(SW_SHUT_DOWN_REG);
    SPI_MasterTransmit(mskSW_SHUT_DOWN_MODE);
    HIGH(PORTB,SS_PIN);

    #ifdef CHECK_ID
    LOW(PORTB,SS_PIN);
    
    // header: read function
    SPI_MasterTransmit(0xAB);
    // check ID returns OK
    // reg 0x1B is ID
    SPI_MasterTransmit(0x1B);
    SPI_MasterTransmit(0x00);
    sled1735_status = SPDR;
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
    
    #ifdef VAF
    // think this has to happen while chip is in shutdown
    setup_vaf();
    #endif

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
    SPI_MasterTransmit(0b10111111); // set to full current
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

    for(int led_frame = 0; led_frame < 2; led_frame ++)
    {
        LOW(PORTB,SS_PIN);
        // header: write frame 1
        SPI_MasterTransmit(0x20 + led_frame); 
        // reg 0x00: led on/off, 1 bit per led, 16 bytes for 128 leds
        SPI_MasterTransmit(0x00); 

        // write 0xFF 16 times to get all 128 LEDs in first frame turned on
        // auto increment means don't need to change start reg
        for(int i=0; i<16; i++)
            SPI_MasterTransmit(0xFF);

        HIGH(PORTB,SS_PIN);
        _delay_ms(1);
    }


    // write 0xFF 16 times to get all 128 LEDs in first frame turned on
    /*
    for(int led_frame = 0; led_frame < 2; led_frame ++)
    {
        LOW(PORTB,SS_PIN);
        // header: write frame 1
        SPI_MasterTransmit(0x20 + led_frame); 
        // reg 0x00: led on/off, 1 bit per led, 16 bytes for 128 leds
        SPI_MasterTransmit(0x00); 
        int led_num = 0;
        while(led_num < 256)
        {
            // build mask
            uint8_t mask = 0xFF;
            for(int i = 0; i < 8; i ++)
            {
                //if led is placed
                if(pgm_read_byte_near(&led_LUT[led_frame][led_num++]) == 0xFF)
                    mask |= 0 << i;
            }

//            SPI_MasterTransmit(mask);
            SPI_MasterTransmit(0xFF);
        }
        HIGH(PORTB,SS_PIN);
    } 
    */

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
    

    #ifdef SELF_TEST
    read_led_open_reg();
    #endif


    #ifdef SPI_INTS
    SPCR |= (1<<SPIE);
    sei();
    #endif
}

void setup_vaf()
{
#ifdef VAF
        /*
        //Setting SLED1735 Ram Page to Function Page            
        // System must go to SW shutdowm mode when initialization
        SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, SW_SHUT_DOWN_REG, mskSW_SHUT_DOWN_MODE);           
        //Setting Matrix Type = Type3   
        SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, PICTURE_DISPLAY_REG, mskMATRIX_TYPE_TYPE3);                
        //Setting Staggered Delay                       
        SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, STAGGERED_DELAY_REG, ((mskSTD4 & CONST_STD_GROUP4)|(mskSTD3 & CONST_STD_GROUP3)|(mskSTD2 & CONST_STD_GROUP2)|(mskSTD1 & CONST_STD_GROUP1)));
        //Enable Slew Rate control 
        SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, SLEW_RATE_CTL_REG, mskSLEW_RATE_CTL_EN);
        */

        
        //===============================================================
        //VAF Control settings base on the LED type.
        //================================================================
        
       // SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, VAF_CTL_REG, (mskVAF2|mskVAF1));

    // consider VAF1,2 and 3 fine tuning, can see the difference on the scope, but the main part is turning it on in the first place.
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0x2B); 
    SPI_MasterTransmit(VAF_CTL_REG); 
    SPI_MasterTransmit(mskVAF2|mskVAF1); 
    HIGH(PORTB,SS_PIN);

       // SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, VAF_CTL_REG2, (mskFORCEVAFCTL_VAFTIMECTL|(mskFORCEVAFTIME_CONST & 0x01)|mskVAF3));

       // this is the crucial configuration...
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0x2B); 
    SPI_MasterTransmit(VAF_CTL_REG2); 
    SPI_MasterTransmit(mskFORCEVAFCTL_VAFTIMECTL|(mskFORCEVAFTIME_CONST & 0x01)|mskVAF3); 
    //SPI_MasterTransmit(mskFORCEVAFCTL_ALWAYSON|mskVAF3); 
    //SPI_MasterTransmit(mskFORCEVAFCTL_DISABLE|mskVAF3); 
    HIGH(PORTB,SS_PIN);
    

        //================================================================
        
        //======================================================//
        //Init Type3 FrameVAFPage : Single R/G/B ,              //
        //Anode RGB or Cathode RGB have different VAF settings  //
        //which can choice by " TYPE2_VAF_OPTION ".             //
        //======================================================//      
        
// vaf is set to vaf2 by default
// this part doesn't seem to make any difference
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0x2D); 
    SPI_MasterTransmit(TYPE3_VAF_FRAME_FIRST_ADDR); 
        for( int i = 0; i< TYPE3_VAF_FRAME_LENGTH ; i++)
        {
                //hwSPI_Tx_Fifo[i+2] = tabLED_Type3Vaf[i];
                SPI_MasterTransmit(tabLED_Type3Vaf[i]); 
        }
    //    SPI_W_NBYTE(SPI_FRAME_LED_VAF_PAGE, TYPE3_VAF_FRAME_FIRST_ADDR, TYPE3_VAF_FRAME_LENGTH);        

    HIGH(PORTB,SS_PIN);

       #endif 

}

void read_led_open_reg()
{

    // make sure test results are off to start with
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0x2B);  // write function reg
    SPI_MasterTransmit(0x11);  // reg 11 - open and short detection status
    SPI_MasterTransmit(0x00);  // 
    HIGH(PORTB,SS_PIN);



    // start the open test
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0x2B);  // write function reg
    SPI_MasterTransmit(0x10);  // reg 10 - open and short detection start
    SPI_MasterTransmit(0b10000001); //start open test. The 1 at the end is the OSDD (open short detection duty). At 63 and 3 I get bad results for leds B1 through P3. At 1 I get the results I expect.
    HIGH(PORTB,SS_PIN);

    while(true)
    {
        // check the test is started
        LOW(PORTB,SS_PIN);
        SPI_MasterTransmit(0xAB);  // read function reg
        SPI_MasterTransmit(0x11);  // reg 10 - open and short detection start
        SPI_MasterTransmit(0x00);  // check status
        HIGH(PORTB,SS_PIN);

        if(SPDR == 0x80)
            break;
        else
            // wait for the test to finish
            _delay_ms(1);
    }
    //////////// debug pin!!!!!!!!!!!!
    HIGH(PORTA,1); // debug pin HIGH
    /* test for sigrok logic analyser
    unsigned int i = 0;
    while(i < 65535)
    {
        SPI_MasterTransmit(i >> 8);  // start at first address
        SPI_MasterTransmit((uint8_t)(i & 0x00FF));  // start at first address
        i++;
        HIGH(PORTA,1); // debug pin HIGH
        _delay_us(50);
        LOW(PORTA,1); // debug pin HIGH
        _delay_us(50);
    }
    */

    // now read all the registers
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0xAC);  // read 0xC
    SPI_MasterTransmit(0x00);  // start at first address
    for(int i=0x0; i<0x20; i++)
    {
        // auto inc addresses
        SPI_MasterTransmit(0x00);  // dummy byte
        led_open_status[i] = SPDR;

    }
    HIGH(PORTB,SS_PIN);

    //////////// debug pin!!!!!!!!!!!!
    LOW(PORTA,1); // debug pin LOW


    // start the short test
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0x2B);  // write function reg
    SPI_MasterTransmit(0x10);  // reg 10 - open and short detection start
    SPI_MasterTransmit(0b01000011); //start short test
    HIGH(PORTB,SS_PIN);


    while(true)
    {
        // check the test is started
        LOW(PORTB,SS_PIN);
        SPI_MasterTransmit(0xAB);  // read function reg
        SPI_MasterTransmit(0x11);  // reg 10 - open and short detection start
        SPI_MasterTransmit(0x00);  // check status
        HIGH(PORTB,SS_PIN);

        // 0x11 should be 0xC0 (as previous test has finished and we didn't reset it
        if(SPDR == 0xC0)
            break;
        else
            // wait for the test to finish
            _delay_ms(1);
    }


    // read the short registers
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0xAC);  // read 0xC
    SPI_MasterTransmit(0x20);  // start at first address
    for(int i=0x0; i<0x20; i++)
    {
        // auto inc addresses
        SPI_MasterTransmit(0x00);  // dummy byte
        led_short_status[i] = SPDR;

    }
    HIGH(PORTB,SS_PIN);

    // have to read this register again, otherwise sled never responds again
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0xAB);  // read function reg
    SPI_MasterTransmit(0x10);  // reg 10 - open and short detection start
    SPI_MasterTransmit(0x00);  // check status
    HIGH(PORTB,SS_PIN);
}

// state machine variables
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
