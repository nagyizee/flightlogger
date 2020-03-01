#ifndef	_OS_INTERNALS_H
#define _OS_INTERNALS_H
/**
 *
 *
 *
 */

/*--------------------------------------------------
 * 		    		Includes
 *--------------------------------------------------*/

#include "base.h"
#include "Os.h"

/*--------------------------------------------------
 * 		    		Defines
 *--------------------------------------------------*/

#define OSINT_DEAD_TIME			(10)		/* nr. of Os timer ticks considered for dead time which is needed for OS housekeeping after each task call */

/*--------------------------------------------------
 * 		    		 Types
 *--------------------------------------------------*/

typedef struct
{
	uint32 task_idx;						/* RT task to be executed at the timer interrupt/event */
	uint32 tstamp_current;					/* Timestamp of the RT task at the index - theoretical value */
	uint32 tstamp_last;						/* timer counter value at the previous task interrupt - real value - used for time counter incrementing */
	tOsTimestamp time_ctr;					/* time counter in os timer ticks (not us) */

} tOsRtInternalInstance;

typedef struct
{
	tOsBgndRunMode run_mode;				/* run mode for the background task */
	tOsTimestamp wakeup_time;				/* timestamp in Os ticks whent to wake up the background task */
	uint32 wakeup_rt_context;				/* non-0 if RT task did a wake-up event (last event recorded only) - used in RT and Bgnd contexts */
} tOsBgndInternalInstance;

/*--------------------------------------------------
 *   		  	 	Functions
 *--------------------------------------------------*/

/* init the system and other runtime stuff - configurable per project */
void OsRt_Init(void);


#endif


