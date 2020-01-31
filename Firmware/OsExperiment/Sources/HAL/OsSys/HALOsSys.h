#ifndef	_HALOSSYS_H
#define _HALOSSYS_H
/*
 *	OsSys is the hardware abstraction layer for Os which
 *	is responsible for timed interrupt generation.
 *	This module is hard coupled with the Os module.
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

#if OSSYS_DBG_ENABLE != 0
	#define OSSYS_DBG_TOGGLE_ON				do { } while(0)
	#define OSSYS_DBG_TOGGLE_OFF			do { } while(0)
#else
	#define OSSYS_DBG_TOGGLE_ON				do { } while(0)
	#define OSSYS_DBG_TOGGLE_OFF			do { } while(0)
#endif


#define HALOsSys_DisableAllInterrupts()		do { __disable_irq(); } while(0)
#define HALOsSys_EnableAllInterrupts()		do { __enable_irq(); } while(0)

#define HALOsSys_GetCurrentCounter()		((uint32)TIM17->CNT)

extern volatile uint32 gHALOsSys_CounterValue;
extern volatile uint32 gHALOsSys_CounterNext;

/*--------------------------------------------------
 *   		  	 	Functions
 *--------------------------------------------------*/

/* Init the hardware stuff for the Os Systems */
void HALOsSys_Init(void);

#endif

