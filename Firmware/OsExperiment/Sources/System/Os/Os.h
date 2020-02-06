#ifndef	_OS_H
#define _OS_H
/**
 *
 *
 *
 */

/*--------------------------------------------------
 * 		    		Includes
 *--------------------------------------------------*/

#include "base.h"

/*--------------------------------------------------
 * 		    		Defines
 *--------------------------------------------------*/

/* wake up reasons for the background task */
#define 	OSBGND_WAKE_REASON_BULK		(0xF1000000)
#define 	OSBGND_WAKE_REASON_RTPER	(0xF2000000)
#define 	OSBGND_WAKE_REASON_TSTAMP	(0xF3000000)
/* if RtAppCtrl is used then the reason is provided byt the RT application.
 * The provided reason must have the MSB = 0 */

/* Background task run modes */
typedef enum
{
	eOsBgnd_bulk = 0,	/* background task is re-run after each exit */
	eOsBgnd_periodRt,	/* background task is run again after each RT task run */
	eOsBgnd_RtAppCtrl,	/* background task is woken up by a real time application module */
	eOsBgnd_timestamp,	/* background task is run at timestamp */
} tOsBgndRunMode;

typedef uint64	tOsTimestamp;	/* Os timestamp in us since power-up */


/*--------------------------------------------------
 *   		  	 	Functions
 *--------------------------------------------------*/

/* Init the operating system */
void Os_Init(void);

/* Main entry point in the operating system */
void Os_Run(void);

/* Wake up the background task from a RT application module
 * Function to be used by Real Time tasks only */
void Os_RtWakeUpBgndTask(uint32 context);

/* Background application run mode selector
 * Function to be used by Background tasks only */
void Os_BgndRunMode(tOsBgndRunMode mode);

/* set the timestamp when the background task will be run again
 * Function to be used by Background tasks only */
void Os_BgndRunTimestamp(tOsTimestamp tstamp);

/* get the current time in us
 * Function to be used by Background tasks only */
tOsTimestamp Os_BgndGetCurrentTime(void);

#endif

