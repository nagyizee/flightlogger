#ifndef	_OS_RT_H
#define _OS_RT_H
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

#define OS_TICKS_PER_US						(3)								/* timer granularity set to 3MHz  - NOTE: synchronize it with the OsSys prescaler */
#define OS_TIME2TICK(timeUs)				(timeUs * OS_TICKS_PER_US)		/* convert time in us -> timer ticks */
#define OS_TASK_TOTAL_CYCLE					(OS_TIME2TICK(5000))			/* cycle time to complete the task list */

#define OSRT_TASK_LIST_LENGTH				(10)

/*--------------------------------------------------
 * 		    		 Types
 *--------------------------------------------------*/

typedef void (*tOsTaskItem)(void);

typedef struct
{
	tOsTaskItem taskItem;
	uint32		timeStamp;
} tOsSchedulerElement;

extern tOsSchedulerElement gOsRt_TaskList[];
extern tOsSchedulerElement gOsRt_BackgndTask;

/*--------------------------------------------------
 *   		  	 	Functions
 *--------------------------------------------------*/



#endif

