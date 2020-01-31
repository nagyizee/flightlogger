/*--------------------------------------------------
 * 		    		Includes
 *--------------------------------------------------*/

#include <string.h>
#include "Os.h"
#include "Os_Internals.h"
#include "Os_Rt.h"
#include "HALOsSys.h"

/*--------------------------------------------------
 * 		    		Defines
 *--------------------------------------------------*/

#define Os_Disable_Interrupts()		do { HALOsSys_DisableAllInterrupts(); } while (0)
#define Os_Enable_Interrupts()		do { HALOsSys_EnableAllInterrupts(); } while (0)

/*--------------------------------------------------
 * 	       Local Variables and prototypes
 *--------------------------------------------------*/

static tOsInternalInstance	lOs;


static void local_InitInternals(void);

/*--------------------------------------------------
 *   		   Interface Functions
 *--------------------------------------------------*/

void Os_Init(void)
{
	HALOsSys_DisableAllInterrupts();
	/* init runtime elements */
	OsRt_Init();
	/* init Os internals */
	local_InitInternals();

	/* init os hardware resources - this will enable the interrupts also */
	HALOsSys_Init();
}

void Os_Run(void)
{
	while (1)
	{
		gOsRt_BackgndTask.taskItem();
	}
}


/*--------------------------------------------------
 *   		   Exported Function
 *--------------------------------------------------*/

void Os_ShedulerTimedTaskEntry(void)
{
	uint32 count;
	uint32 next_timestamp;
	uint32 time_diff;
	/* gHALOsSys_CounterValue - is the counter value when the timer interrupt is produced
	 *
	 */

	/*-----  execute the task at the set index ----*/
    gOsRt_TaskList[lOs.task_idx].taskItem();
    /*----- check the time spent in the task -----*/
    count = HALOsSys_GetCurrentCounter();
    time_diff = gOsRt_TaskList[lOs.task_idx].timeStamp;

    /* get the time difference (consider the wraparround) */
    if (count <= gHALOsSys_CounterValue)
    {
    	count = OSSYS_TIMER_MAX_CYCLES - gHALOsSys_CounterValue + count;
    }
    else
    {
    	count = count - gHALOsSys_CounterValue;
    }

    /* go to the next task from list and calculate the time difference bw. the two tasks */
    lOs.task_idx++;
    if (lOs.task_idx == OSRT_TASK_LIST_LENGTH)
    {
    	lOs.task_idx = 0;
    	time_diff = OS_TASK_TOTAL_CYCLE - time_diff;
    }
    else
    {
    	time_diff = gOsRt_TaskList[lOs.task_idx].timeStamp - time_diff;
    }

    //TODO: overtime protection and time recovery mechanism //

    /* set up the next timestamp */
    next_timestamp = lOs.tstamp_current + time_diff;
    if (next_timestamp >= OSSYS_TIMER_MAX_CYCLES)
    {
    	next_timestamp = next_timestamp - OSSYS_TIMER_MAX_CYCLES;
    }
    gHALOsSys_CounterNext = next_timestamp;
    lOs.tstamp_current = next_timestamp;
}


/*--------------------------------------------------
 *   		   Local Functions
 *--------------------------------------------------*/

static void local_InitInternals(void)
{
	/* reset the internal structure */
	memset(&lOs, 0, sizeof(lOs));
	lOs.tstamp_current = gOsRt_TaskList[0].timeStamp;
	/* set up the first time event to trigger the first task */
	gHALOsSys_CounterNext = gOsRt_TaskList[0].timeStamp;
}

