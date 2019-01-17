#pragma once
#include <stdint.h>
void sled_test();
void setup_spi();
void setup_vaf();
void self_test(uint8_t osdd);

uint8_t led_open_status[32];
uint8_t led_short_status[32];
uint8_t sled1735_status;
uint8_t sled1735_const_current;


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


//check masks -depend on package?
#define	mskVAF1									(0x7<<0) 		
#define	mskVAF2									(0x0<<4)		
#define	mskVAF3									(0x0<<0) 		

#define	mskFORCEVAFTIME_CONST			        (0x0<<3) 			
#define	mskFORCEVAFCTL_ALWAYSON		            (0x0<<6) 	
#define	mskFORCEVAFCTL_VAFTIMECTL	            (0x1<<6) 	
#define	mskFORCEVAFCTL_DISABLE		            (0x2<<6) 	

#define mskMATRIX_TYPE_TYPE3                    (0x2<<3) 

#define mskSW_SHUT_DOWN_MODE                    (0x0<<0)
#define mskSW_NORMAL_MODE                       (0x1<<0)

#define mskSLEW_RATE_CTL_EN                     (0x1<<0)
#define mskSLEW_RATE_CTL_DIS                    (0x0<<0)

/*--------Function Register: address 0DH Staggered Delay Reg------------*/      
#define mskSTD1                                 (0x3<<0)        
#define mskSTD2                                 (0x3<<2)
#define mskSTD3                                 (0x3<<4)        
#define mskSTD4                                 (0x3<<6)

#define CONST_STD_GROUP1                        0x00            
#define CONST_STD_GROUP2                        0x55    
#define CONST_STD_GROUP3                        0xAA            
#define CONST_STD_GROUP4                        0xFF            



#define mskCURRENT_STEP_CONST                   (0x3F<<0)               
#define CONST_CURRENT_STEP_10mA                 (0x5<<0)
#define CONST_CURRENT_STEP_15mA                 (0xF<<0)
#define CONST_CURRENT_STEP_20mA                 (0x19<<0)
#define CONST_CURRENT_STEP_25mA                 (0x23<<0)               
#define CONST_CURRENT_STEP_32mA                 (0x31<<0)
#define CONST_CURRENT_STEP_40mA                 (0x3F<<0)

#define mskCURRENT_CTL_EN                       (0x1<<7)        
#define mskCURRENT_CTL_DIS                      (0x0<<7)  

#define mskOPEN_DETECT_START                    (0x1<<7) // HW will clear this bits automatically
#define mskSHORT_DETECT_START                   (0x1<<6) // HW will clear this bits automatically 

#define mskSHORT_DETECT_INT                     (0x1<<6) //when open/short detect done, HW will set this bit as 1 automatically 
#define mskOPEN_DETECT_INT                      (0x1<<7) //when open/short detect done, HW will set this bit as 1 automatically   

#define	TYPE3_VAF_FRAME_FIRST_ADDR				0x00	
#define	TYPE3_VAF_FRAME_LAST_ADDR				0x3F								 	
#define	TYPE3_VAF_FRAME_LENGTH					((TYPE3_VAF_FRAME_LAST_ADDR-TYPE3_VAF_FRAME_FIRST_ADDR)+1)

#define TYPE3_PWM_FRAME_FIRST_ADDR              0x20    
#define TYPE3_PWM_FRAME_LAST_ADDR               0x9F                                                                    
#define TYPE3_PWM_FRAME_LENGTH                  ((TYPE3_PWM_FRAME_LAST_ADDR-TYPE3_PWM_FRAME_FIRST_ADDR)+1) 

#define TYPE3_LED_FRAME_FIRST_ADDR              0x00    
#define TYPE3_LED_FRAME_LAST_ADDR               0x0F                                                                    
#define TYPE3_LED_FRAME_LENGTH                  ((TYPE3_LED_FRAME_LAST_ADDR-TYPE3_LED_FRAME_FIRST_ADDR)+1)


#define	SPI_FRAME_ONE_PAGE				        0x0			//Setting SLED1735 Frame Page 1
#define	SPI_FRAME_TWO_PAGE			    	    0x1			//Setting SLED1735 Frame Page 2	
#define	SPI_FRAME_FUNCTION_PAGE		            0xB			//Setting SLED1735 Frame Page 9	
#define	SPI_FRAME_DETECTION_PAGE	            0xC			//Setting SLED1735 Frame Page 10
#define	SPI_FRAME_LED_VAF_PAGE		            0xD			//Setting SLED1735 Frame Page 11	



#define CONFIGURATION_REG                       0x00
#define PICTURE_DISPLAY_REG                     0x01
#define DISPLAY_OPTION_REG                      0x05
#define BREATH_CTL_REG                          0x08
#define BREATH_CTL_REG2                         0x09
#define SW_SHUT_DOWN_REG                        0x0A   
#define AUDIO_GAIN_CTL_REG                      0x0B
#define STAGGERED_DELAY_REG                     0x0D
#define SLEW_RATE_CTL_REG                       0x0E
#define CURRENT_CTL_REG                         0x0F
#define OPEN_SHORT_REG                          0x10                             
#define OPEN_SHORT_REG2                         0x11                    
#define VAF_CTL_REG                             0x14                        
#define VAF_CTL_REG2                            0x15       
#define TYPE4_VAF_REG1                          0x18     
#define TYPE4_VAF_REG2                          0x19
#define TYPE4_VAF_REG3                          0x1A
#define CHIP_ID_REG                             0x1B

