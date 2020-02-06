#ifndef	_HALOSSYS_H
#define _HALOSSYS_H
/*
 *	OsSys is the hardware abstraction layer for Os which
 *	is responsible for timed interrupt generation.
 *	This module is hard coupled with the Os module.
 *
 *	Interfaces:
 *		gHALOsSys_CounterValue [OsSys -> Os]: timer counter value when the scheduler interrupt was produced
 *		gHALOsSys_CounterNext  [OsSys <- Os]: next scheduled wake-up set by the Os.
 *		HALOsSys_GetCurrentCounter():		  gets the os timer counter value
 *		HALOsSys_DisableAllInterrupts():	  disable the global interrupt handling
 *		HALOsSys_EnableAllInterrupts():		  enable the global interrupt handling
 *
 */

/*--------------------------------------------------
 * 		    		Includes
 *--------------------------------------------------*/

#include "stm32f100xb.h"
#include "base.h"

/*--------------------------------------------------
 * 		    		Defines
 *--------------------------------------------------*/

#define OSSYS_TIMER_MAX_CYCLES				(0x10000)

#define OSSYS_DBG_ENABLE					(1)

#define HALOsSys_DisableAllInterrupts()		do { __disable_irq(); } while(0)
#define HALOsSys_EnableAllInterrupts()		do { __enable_irq(); } while(0)

#define HALOsSys_Sleep()					do { asm("wfi"); } while (0)

#define HALOsSys_GetCurrentCounter()		((uint32)TIM17->CNT)

extern volatile uint32 gHALOsSys_CounterValue;
extern volatile uint32 gHALOsSys_CounterNext;

/*--------------------------------------------------
 *   		  	 	Functions
 *--------------------------------------------------*/

/* Init the hardware stuff for the Os Systems */
void HALOsSys_Init(void);

#endif

