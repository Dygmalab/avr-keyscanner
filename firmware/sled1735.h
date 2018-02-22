#pragma once
#include <stdint.h>
void sled_test();
void setup_spi();
void setup_vaf();

uint8_t led_open_status[32];
uint8_t led_short_status[32];
uint8_t sled1735_status;


/* Call to begin transmitting LED data */

//void led_init(void);
//void led_update(void);

/* Call to change the speed at which we update the LEDs */
//void led_set_spi_frequency(uint8_t frequency);

/* Call this when you have new preformatted data for the LEDs */
void led_update_bank(uint8_t *buf, const uint8_t bank);

/* Call this when you want to set only a single LED */
void led_set_one_to(uint8_t led, uint8_t * led_data);

/* Call this when you want to set every LED to the same value */
void led_set_all_to(uint8_t * led_data);

/* Call this with a value between 0 and 31 to set the LED's global brightness */
//void led_set_global_brightness(uint8_t global_brightness);

void read_led_open_reg();

            //check masks -depend on package?
            #define	mskVAF1										(0x7<<0) 		
            #define	mskVAF2										(0x0<<4)		
            #define	mskVAF3										(0x0<<0) 		

            #define	mskFORCEVAFTIME_CONST			(0x0<<3) 			
            #define	mskFORCEVAFCTL_ALWAYSON		(0x0<<6) 	
            #define	mskFORCEVAFCTL_VAFTIMECTL	(0x1<<6) 	
            #define	mskFORCEVAFCTL_DISABLE		(0x2<<6) 	

            #define	VAF_CTL_REG 0X14			
            #define	VAF_CTL_REG2 0X15	

            #define	LED_VAF_PAGE 0X0D

			#define	TYPE3_VAF_FRAME_FIRST_ADDR					0x00	
			#define	TYPE3_VAF_FRAME_LAST_ADDR						0x3F								 	
			#define	TYPE3_VAF_FRAME_LENGTH					((TYPE3_VAF_FRAME_LAST_ADDR-TYPE3_VAF_FRAME_FIRST_ADDR)+1)

        #define		SPI_FRAME_ONE_PAGE				0x0			//Setting SLED1735 Frame Page 1
        #define		SPI_FRAME_TWO_PAGE				0x1			//Setting SLED1735 Frame Page 2	
        #define		SPI_FRAME_FUNCTION_PAGE		0xB			//Setting SLED1735 Frame Page 9	
        #define		SPI_FRAME_DETECTION_PAGE	0xC			//Setting SLED1735 Frame Page 10
        #define		SPI_FRAME_LED_VAF_PAGE		0xD			//Setting SLED1735 Frame Page 11	

        #define mskSW_SHUT_DOWN_MODE                                    (0x0<<0)
		#define	SW_SHUT_DOWN_REG 0X0A	
