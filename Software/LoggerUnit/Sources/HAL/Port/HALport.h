#ifndef	_HALPORT_H
#define _HALPORT_H
/**
 *
 *
 *
 */

/*--------------------------------------------------
 * 		    		Includes
 *--------------------------------------------------*/

#include "nrf.h"

/*--------------------------------------------------
 * 		    		Defines
 *--------------------------------------------------*/

/* define GPIO pins used in the project */
#define		PIN_TESTPAD_0		(0)         /* gen.out */
#define 	PIN_TESTPAD_1		(1)         /* gen.out */
#define 	PIN_I2C_SCK  	    (2)         /* i2c */
#define 	PIN_I2C_SD	    	(3)         /* i2c */
#define     PIN_ANA_PWRSRC      (4)         /* analog.in */
#define     PIN_ANA_EXT         (5)         /* analog.in */
#define     PIN_PPM_IN          (6)         /* tim.capt.in */
#define     PIN_ACC_INT         (7)         /* gen.in */
#define     PIN_IF_TX_OUT       (8)         /* uart */
#define     PIN_IF_RX_IN        (9)         /* uart */
#define     PIN_IF_CTRL         (10)        /* gen.out */
#define     PIN_LED_ON          (17)        /* gen.out */
#define     PIN_LED_BLE         (19)        /* gen.out */
#define     PIN_FLS_RESET       (26)        /* gen.out */
#define     PIN_FLS_CLK         (27)        /* spi */
#define     PIN_FLS_SI          (28)        /* spi */
#define     PIN_FLS_SO          (29)        /* spi */
#define     PIN_FLS_CS          (30)        /* gen.out */
#define     PIN_PRESS_INT       (31)        /* gen.in */

#define     GPIO_TESTPAD_0       NRF_P0
#define     GPIO_TESTPAD_1       NRF_P0
#define     GPIO_I2C_SCK         NRF_P0
#define     GPIO_I2C_SD          NRF_P0
#define     GPIO_ANA_PWRSRC      NRF_P0
#define     GPIO_ANA_EXT         NRF_P0
#define     GPIO_PPM_IN          NRF_P0
#define     GPIO_ACC_INT         NRF_P0
#define     GPIO_IF_TX_OUT       NRF_P0
#define     GPIO_IF_RX_IN        NRF_P0
#define     GPIO_IF_CTRL         NRF_P0
#define     GPIO_LED_ON          NRF_P0
#define     GPIO_LED_BLE         NRF_P0
#define     GPIO_FLS_RESET       NRF_P0
#define     GPIO_FLS_CLK         NRF_P0
#define     GPIO_FLS_SI          NRF_P0
#define     GPIO_FLS_SO          NRF_P0
#define     GPIO_FLS_CS          NRF_P0
#define     GPIO_PRESS_INT       NRF_P0

/* Generic Output pin operations */
#define 	PORT_PIN_TESTPAD_0_ON()		    do { GPIO_PIN_TESTPAD_0->OUTSET  = (1U << PIN_TESTPAD_0); } while(0)
#define 	PORT_PIN_TESTPAD_0_OFF()		do { GPIO_PIN_TESTPAD_0->OUTCLR  = (1U << PIN_TESTPAD_0); } while(0)

#define     PORT_PIN_TESTPAD_1_ON()         do { GPIO_PIN_TESTPAD_1->OUTSET  = (1U << PIN_TESTPAD_1); } while(0)
#define     PORT_PIN_TESTPAD_1_OFF()        do { GPIO_PIN_TESTPAD_1->OUTCLR  = (1U << PIN_TESTPAD_1); } while(0)

#define     PORT_PIN_IF_CTRL_ON()           do { GPIO_PIN_IF_CTRL->OUTSET  = (1U << PIN_IF_CTRL); } while(0)
#define     PORT_PIN_IF_CTRL_OFF()          do { GPIO_PIN_IF_CTRL->OUTCLR  = (1U << PIN_IF_CTRL); } while(0)

#define     PORT_PIN_LED_ON_ON()            do { GPIO_LED_ON->OUTSET  = (1U << PIN_LED_ON); } while(0)
#define     PORT_PIN_LED_ON_OFF()           do { GPIO_LED_ON->OUTCLR  = (1U << PIN_LED_ON); } while(0)

#define     PORT_PIN_LED_BLE_ON()           do { GPIO_PIN_LED_BLE->OUTSET  = (1U << PIN_LED_BLE); } while(0)
#define     PORT_PIN_LED_BLE_OFF()          do { GPIO_PIN_LED_BLE->OUTCLR  = (1U << PIN_LED_BLE); } while(0)

#define     PORT_PIN_FLS_RESET_ON()         do { GPIO_PIN_FLS_RESET->OUTSET  = (1U << PIN_FLS_RESET); } while(0)
#define     PORT_PIN_FLS_RESET_OFF()        do { GPIO_PIN_FLS_RESET->OUTCLR  = (1U << PIN_FLS_RESET); } while(0)

#define     PORT_PIN_FLS_CS_ON()            do { GPIO_PIN_FLS_CS->OUTSET  = (1U << PIN_FLS_CS); } while(0)
#define     PORT_PIN_FLS_CS_OFF()           do { GPIO_PIN_FLS_CS->OUTCLR  = (1U << PIN_FLS_CS); } while(0)

/* Generic Input pin operations */


/*--------------------------------------------------
 *   		  	 	Functions
 *--------------------------------------------------*/

/* Init all the GPIO and Alternate function related stuff */
void HALPort_Init(void);


#endif

