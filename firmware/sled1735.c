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

// functionality
#define CONST_CURR
#define SPI_INTS
//#define SELF_TEST
#define VAF
#define INIT_PWM 0x00

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

// state machine variables for SPI update of sled1735 LED buffer
uint8_t volatile led_num = 0;
uint8_t volatile led_frame = 0;
uint8_t volatile led_pos = 0;
static volatile enum { BANK, REG, DATA, END } led_state;


#define LED1 0, 0, 0
#define LED4 LED1, LED1, LED1, LED1
#define LED8 LED4, LED4
#define LED16 LED8, LED8
#define LED32 LED16, LED16
#define LED64 LED32, LED32
#define LED128 LED64, LED64

led_buffer_t led_buffer = { LED64, LED8 };

#ifdef VAF
//Reference SLED1735 Datasheet Type3 Circuit Map
const uint8_t tabLED_Type3Vaf[64] = { 
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

// pin defs
#define SHUTDOWN_PIN 6 //shutdown when low
#define SS_PIN 2 // 7 for matrix test board, 2 for raise
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

// when writing to sled1735:
// 1st byte split into: 4bits is 0xA read, 0x2 write
//                      4bits is the page
// 2nd byte is the register to write/read
void SPI_W_3BYTE(uint8_t page, uint8_t reg, uint8_t data)
{
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0x20 + page);
    SPI_MasterTransmit(reg);
    SPI_MasterTransmit(data);
    HIGH(PORTB,SS_PIN);
}

uint8_t SPI_R_3BYTE(uint8_t page, uint8_t reg)
{
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0xA0 + page);
    SPI_MasterTransmit(reg);
    SPI_MasterTransmit(0x00); // dummy byte
    HIGH(PORTB,SS_PIN);
    return SPDR;
}

void led_update_bank(uint8_t *buf, const uint8_t bank) 
{

    memcpy(&led_buffer.bank[bank], buf, LED_BANK_SIZE);
}

void led_set_one_to(uint8_t led, uint8_t *buf) 
{
    //overflow possible here
    memcpy(&led_buffer.weach[led], buf, LED_DATA_SIZE);
}

void led_set_all_to( uint8_t *buf) 
{
    for(uint8_t led=0; led <NUM_LEDS; led++) 
    {
        memcpy(&led_buffer.weach[led], buf, LED_DATA_SIZE);
    }
}


void setup_spi()
{
    SPI_MasterInit(); 

    // shutdown - chip must be shutdown to do a lot of the setup
    SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, SW_SHUT_DOWN_REG, mskSW_SHUT_DOWN_MODE);           

    // get the chip's ID - this is fetchable over I2C interface
    sled1735_status = SPI_R_3BYTE(SPI_FRAME_FUNCTION_PAGE, CHIP_ID_REG);

    // enable picture mode, disable ADC
    SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, CONFIGURATION_REG, 0x00);           

    // matrix type 3 - 70 RGB common anode
    SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, PICTURE_DISPLAY_REG, mskMATRIX_TYPE_TYPE3);           

    // Setting Staggered Delay - no noticable effect
    //SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, STAGGERED_DELAY_REG, ((mskSTD4 & CONST_STD_GROUP4)|(mskSTD3 & CONST_STD_GROUP3)|(mskSTD2 & CONST_STD_GROUP2)|(mskSTD1 & CONST_STD_GROUP1)));
    
    // Enable Slew Rate control 
    SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, SLEW_RATE_CTL_REG, mskSLEW_RATE_CTL_EN);
    
    // vaf fixes dim red LEDs that should be off
    #ifdef VAF
    SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, VAF_CTL_REG, (mskVAF2|mskVAF1));
    SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, VAF_CTL_REG2, (mskFORCEVAFCTL_VAFTIMECTL|(mskFORCEVAFTIME_CONST & 0x01)|mskVAF3));

    // vaf is set to vaf2 by default
    // this part doesn't seem to make any visible difference, only visible on the scope
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0x20 + SPI_FRAME_LED_VAF_PAGE); 
    SPI_MasterTransmit(TYPE3_VAF_FRAME_FIRST_ADDR); 
    for( int i = 0; i< TYPE3_VAF_FRAME_LENGTH ; i++)
        SPI_MasterTransmit(tabLED_Type3Vaf[i]); 
    HIGH(PORTB,SS_PIN);
    #endif 

    #ifdef CONST_CURR
    SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, CURRENT_CTL_REG, mskCURRENT_CTL_EN | CONST_CURRENT_STEP_40mA);           
    #endif

    #ifdef INIT_PWM
    // initialise pwm to our default value
    for(int i=TYPE3_PWM_FRAME_FIRST_ADDR; i<=TYPE3_PWM_FRAME_LAST_ADDR; i++)
        SPI_W_3BYTE(SPI_FRAME_ONE_PAGE, i, INIT_PWM);           

    for(int i=TYPE3_PWM_FRAME_FIRST_ADDR; i<=TYPE3_PWM_FRAME_LAST_ADDR; i++)
        SPI_W_3BYTE(SPI_FRAME_TWO_PAGE, i, INIT_PWM);           
    #endif

    // turn on all leds - alternative is to only turn on those that are set in the LUT
    for(int i=TYPE3_LED_FRAME_FIRST_ADDR; i<=TYPE3_LED_FRAME_LAST_ADDR; i++)
        SPI_W_3BYTE(SPI_FRAME_ONE_PAGE, i, 0xFF);           

    for(int i=TYPE3_LED_FRAME_FIRST_ADDR; i<=TYPE3_LED_FRAME_LAST_ADDR; i++)
        SPI_W_3BYTE(SPI_FRAME_TWO_PAGE, i, 0xFF);           

    // turn on the chip
    SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, SW_SHUT_DOWN_REG, mskSW_NORMAL_MODE);           
    
    #ifdef SELF_TEST
    // run the self test to get list of opens and shorts - use this after assembly to aid in checking leds
    self_test();
    #endif

    #ifdef SPI_INTS
    // turn on spi interrupts to start automatic update of sled1735's led ram.
    SPCR |= (1<<SPIE);
    sei();
    #endif
}

void self_test()
{
    // make sure test results are off to start with
    SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, OPEN_SHORT_REG2, 0x00);           

    // OSDD = open short detection duty - don't know how it works. At 63 and 3 I get bad results for leds B1 through P3. At 1 I get the results I expect.
    uint8_t osdd = 0x01;

    // start open test
    SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, OPEN_SHORT_REG, mskOPEN_DETECT_START + osdd);           

    // wait for test to run
    while(SPI_R_3BYTE(SPI_FRAME_FUNCTION_PAGE, OPEN_SHORT_REG2) != mskOPEN_DETECT_INT)
        _delay_ms(1);

    // read all open registers
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0xA0 + SPI_FRAME_DETECTION_PAGE);
    SPI_MasterTransmit(0x00);  // start at first address
    for(int i=0x0; i<0x20; i++)
    {
        SPI_MasterTransmit(0x00);  // dummy byte
        led_open_status[i] = SPDR;
    }
    HIGH(PORTB,SS_PIN);

    // start short circuit test, set results to 0 again
    SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, OPEN_SHORT_REG2, 0x00);           
    SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, OPEN_SHORT_REG, mskSHORT_DETECT_START + osdd);

    // wait for test to run
    while(SPI_R_3BYTE(SPI_FRAME_FUNCTION_PAGE, OPEN_SHORT_REG2) != mskSHORT_DETECT_INT)
        _delay_ms(1);

    // read the short registers
    LOW(PORTB,SS_PIN);
    SPI_MasterTransmit(0xA0 + SPI_FRAME_DETECTION_PAGE);
    SPI_MasterTransmit(0x20);  // start at first address
    for(int i=0x0; i<0x20; i++)
    {
        SPI_MasterTransmit(0x00);  // dummy byte
        led_short_status[i] = SPDR;
    }
    HIGH(PORTB,SS_PIN);

    // this shouldn't be necessary, but otherwise chip never responds again
    SPI_W_3BYTE(SPI_FRAME_FUNCTION_PAGE, OPEN_SHORT_REG2, 0x00);           
}


// continuously transmit the contents of the led_buffer
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
