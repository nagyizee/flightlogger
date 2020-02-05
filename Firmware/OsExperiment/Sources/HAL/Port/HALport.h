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

#include "stm32f100xb.h"

/*--------------------------------------------------
 * 		    		Defines
 *--------------------------------------------------*/

/* define GPIO pins used in the project */
#define		PIN_BGNDAPP_OUT			(10)
#define 	PIN_RTAPP_OUT			(12)
#define 	PIN_INTERRUPT_1_OUT  	(14)
#define 	PIN_INTERRUPT_2_OUT		(15)

#define 	GPIO_PIN_BGNDAPP_OUT	GPIOB
#define 	GPIO_RTAPP_OUT			GPIOB
#define 	GPIO_INTERRUPT_1_OUT  	GPIOB
#define 	GPIO_INTERRUPT_2_OUT	GPIOB

#define 	PORT_PIN_BGNDAPP_ON()		do { GPIO_PIN_BGNDAPP_OUT->BSRR = (1U << PIN_BGNDAPP_OUT); } while(0)
#define 	PORT_PIN_BGNDAPP_OFF()		do { GPIO_PIN_BGNDAPP_OUT->BRR  = (1U << PIN_BGNDAPP_OUT); } while(0)

#define 	PORT_PIN_RTAPP_ON()	    	do { GPIO_RTAPP_OUT->BSRR = (1U << PIN_RTAPP_OUT); } while(0)
#define 	PORT_PIN_RTAPP_OFF()		do { GPIO_RTAPP_OUT->BRR  = (1U << PIN_RTAPP_OUT); } while(0)

#define 	PORT_PIN_INTR1_ON()			do { GPIO_INTERRUPT_1_OUT->BSRR = (1U << PIN_INTERRUPT_1_OUT); } while(0)
#define 	PORT_PIN_INTR1_OFF()		do { GPIO_INTERRUPT_1_OUT->BRR  = (1U << PIN_INTERRUPT_1_OUT); } while(0)

#define 	PORT_PIN_INTR2_ON()			do { GPIO_INTERRUPT_2_OUT->BSRR = (1U << PIN_INTERRUPT_2_OUT); } while(0)
#define 	PORT_PIN_INTR2_OFF()		do { GPIO_INTERRUPT_2_OUT->BRR  = (1U << PIN_INTERRUPT_2_OUT); } while(0)

/*--------------------------------------------------
 *   		  	 	Functions
 *--------------------------------------------------*/

/* Init all the GPIO and Alternate function related stuff */
void HALPort_Init(void);


#endif

