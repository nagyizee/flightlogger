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

/* variables operated in the RT task context */
static tOsRtInternalInstance	lOsRt;
uint32 							lOsRtHasRun;

/* variables operated in the background task context */
static tOsBgndInternalInstance  lOsBgnd;

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

	/* init os hardware resources - this will enable the interrupts also
	 * IMPORTANT NOTE - context sensitive variables must be operated with precaution after this moment */
	HALOsSys_Init();
}

void Os_Run(void)
{
	/* Code executed in backgnd task context */
	while (1)
	{
		if (lOsBgnd.run_mode == eOsBgnd_bulk)
		{
			/* enter the backgnd task unconditionally */
			gOsRt_BgndTask(OSBGND_WAKE_REASON_BULK);
		}
		else if (lOsBgnd.run_mode == eOsBgnd_periodRt)
		{
			lOsRtHasRun = 0;
			HALOsSys_Sleep();
			/* enter in the background task only if the interrupt was for RT tasks */
			if (lOsRtHasRun)
			{
				gOsRt_BgndTask(OSBGND_WAKE_REASON_RTPER);
			}
		}
		else if (lOsBgnd.run_mode == eOsBgnd_periodRt)
		{

		}
		else	/* eOsBgnd_timestamp */
		{

		}
	}
}


void Os_RtWakeUpBgndTask(uint32 context)
{

}

void Os_BgndRunMode(tOsBgndRunMode mode)
{
	lOsBgnd.run_mode = mode;
}

void Os_BgndRunTimestamp(tOsTimestamp tstamp)
{

}

tOsTimestamp Os_BgndGetCurrentTime(void)
{
	return 0LL;
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
    gOsRt_TaskList[lOsRt.task_idx].taskItem();
    /*----- check the time spent in the task -----*/
    count = HALOsSys_GetCurrentCounter();
    time_diff = gOsRt_TaskList[lOsRt.task_idx].timeStamp;

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
    lOsRt.task_idx++;
    if (lOsRt.task_idx == OSRT_TASK_LIST_LENGTH)
    {
    	lOsRt.task_idx = 0;
    	time_diff = OS_TASK_TOTAL_CYCLE - time_diff;
    }
    else
    {
    	time_diff = gOsRt_TaskList[lOsRt.task_idx].timeStamp - time_diff;
    }

    //TODO: overtime protection and time recovery mechanism //

    /* set up the next timestamp */
    next_timestamp = lOsRt.tstamp_current + time_diff;
    if (next_timestamp >= OSSYS_TIMER_MAX_CYCLES)
    {
    	next_timestamp = next_timestamp - OSSYS_TIMER_MAX_CYCLES;
    }
    gHALOsSys_CounterNext = next_timestamp;
    lOsRtHasRun = ~0;
    lOsRt.tstamp_current = next_timestamp;
}


/*--------------------------------------------------
 *   		   Local Functions
 *--------------------------------------------------*/

static void local_InitInternals(void)
{
	/* reset the internal structure */
	memset(&lOsRt, 0, sizeof(lOsRt));
	memset(&lOsBgnd, 0, sizeof(lOsBgnd));

	lOsRtHasRun = 0;
	lOsRt.tstamp_current = gOsRt_TaskList[0].timeStamp;
	/* set up the first time event to trigger the first task */
	gHALOsSys_CounterNext = gOsRt_TaskList[0].timeStamp;
}

