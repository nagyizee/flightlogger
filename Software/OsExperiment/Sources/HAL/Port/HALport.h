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
#define		PIN_BGNDAPP_OUT			(2)
#define 	PIN_RTAPP_OUT			(3)
#define 	PIN_INTERRUPT_1_OUT  	(5)
#define 	PIN_INTERRUPT_2_OUT		(18)

#define 	GPIO_PIN_BGNDAPP_OUT	NRF_P0
#define 	GPIO_RTAPP_OUT			NRF_P0
#define 	GPIO_INTERRUPT_1_OUT  	NRF_P0
#define 	GPIO_INTERRUPT_2_OUT	NRF_P0

#define 	PORT_PIN_BGNDAPP_ON()		do { GPIO_PIN_BGNDAPP_OUT->OUTSET  = (1U << PIN_BGNDAPP_OUT); } while(0)
#define 	PORT_PIN_BGNDAPP_OFF()		do { GPIO_PIN_BGNDAPP_OUT->OUTCLR  = (1U << PIN_BGNDAPP_OUT); } while(0)

#define 	PORT_PIN_RTAPP_ON()	    	do { GPIO_RTAPP_OUT->OUTSET  = (1U << PIN_RTAPP_OUT); } while(0)
#define 	PORT_PIN_RTAPP_OFF()		do { GPIO_RTAPP_OUT->OUTCLR  = (1U << PIN_RTAPP_OUT); } while(0)

#define 	PORT_PIN_INTR1_ON()			do { GPIO_INTERRUPT_1_OUT->OUTSET  = (1U << PIN_INTERRUPT_1_OUT); } while(0)
#define 	PORT_PIN_INTR1_OFF()		do { GPIO_INTERRUPT_1_OUT->OUTCLR  = (1U << PIN_INTERRUPT_1_OUT); } while(0)

#define 	PORT_PIN_INTR2_ON()			do { GPIO_INTERRUPT_2_OUT->OUTSET  = (1U << PIN_INTERRUPT_2_OUT); } while(0)
#define 	PORT_PIN_INTR2_OFF()		do { GPIO_INTERRUPT_2_OUT->OUTCLR  = (1U << PIN_INTERRUPT_2_OUT); } while(0)

/*--------------------------------------------------
 *   		  	 	Functions
 *--------------------------------------------------*/

/* Init all the GPIO and Alternate function related stuff */
void HALPort_Init(void);


#endif

