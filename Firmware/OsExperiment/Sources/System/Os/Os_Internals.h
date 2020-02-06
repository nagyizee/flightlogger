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

#define OSINT_DEAD_TIME			(10)		/* nr. of Os timer cycles considered for dead time which is needed for OS housekeeping after each task call */

/*--------------------------------------------------
 * 		    		 Types
 *--------------------------------------------------*/

typedef struct
{
	uint32 task_idx;						/* RT task index which is executed at the next interrupt */
	uint32 tstamp_current;					/* Timestamp when the RT task index will be executed */

} tOsRtInternalInstance;

typedef struct
{
	tOsBgndRunMode run_mode;				/* run mode for the background task */

} tOsBgndInternalInstance;

/*--------------------------------------------------
 *   		  	 	Functions
 *--------------------------------------------------*/

/* init the system and other runtime stuff - configurable per project */
void OsRt_Init(void);


#endif


