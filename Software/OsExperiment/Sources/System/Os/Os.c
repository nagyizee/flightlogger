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

/* disable and enable the rt. task interrupts only */
#define Os_Disable_RT_Interrupt()	do { HALOsSys_DisableAllInterrupts(); } while (0)
#define Os_Enable_RT_Interrupt()	do { HALOsSys_EnableAllInterrupts(); } while (0)

/*--------------------------------------------------
 * 	       Local Variables and prototypes
 *--------------------------------------------------*/

/* variables operated in the RT task context */
static tOsRtInternalInstance	lOsRt;
uint32 							lOsRtWake_TaskHasRun;
uint32							lOsRtWake_TaskEvent;
uint32							lOsRtWake_Timestamp;

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
			lOsRtWake_TaskHasRun = 0;		/* no need for mutex - RT task interrupt may overwrite it */
			HALOsSys_Sleep();
			/* enter in the background task only if the interrupt was for RT tasks */
			if (lOsRtWake_TaskHasRun)
			{
				gOsRt_BgndTask(OSBGND_WAKE_REASON_RTPER);
			}
		}
		else if (lOsBgnd.run_mode == eOsBgnd_RtAppCtrl)
		{
			volatile uint32 wake_context;
			HALOsSys_Sleep();
			Os_Disable_RT_Interrupt();
			wake_context = lOsBgnd.wakeup_rt_context;
			lOsBgnd.wakeup_rt_context = 0;
			Os_Enable_RT_Interrupt();
			if (wake_context)
			{
				gOsRt_BgndTask(wake_context);
			}

		}
		else	/* eOsBgnd_timestamp */
		{
			tOsTimestamp crt_time;
			HALOsSys_Sleep();
			Os_Disable_RT_Interrupt();
			crt_time = lOsRt.time_ctr;
			Os_Enable_RT_Interrupt();
			if (lOsBgnd.wakeup_time <= crt_time)
			{
				gOsRt_BgndTask(OSBGND_WAKE_REASON_TSTAMP);
			}
		}
	}
}


void Os_RtWakeUpBgndTask(uint32 context)
{
	/* RT task context */
	if (lOsBgnd.wakeup_rt_context == 0)
	{
		lOsBgnd.wakeup_rt_context = context;		/* no mutex here - we are in interrupt context */
	}

}

tOsTimestamp Os_RtGetCurrentTime(void)
{
	/* RT task context */
	uint32 last_time;
	uint32 crt_time;
	tOsTimestamp total_time;

	crt_time = HALOsSys_GetCurrentCounter();
	last_time = lOsRt.tstamp_last;
	total_time = lOsRt.time_ctr;
    if (last_time > crt_time)
    {
    	total_time += (uint64)(OSSYS_TIMER_MAX_CYCLES - last_time + crt_time);
    }
    else
    {
    	total_time += (uint64)(crt_time - last_time);
    }

    return (total_time / OS_TICKS_PER_US);
}


void Os_BgndRunMode(tOsBgndRunMode mode)
{
	/* Bgnd task context */
	lOsBgnd.run_mode = mode;
}

void Os_BgndRunTimestamp(tOsTimestamp tstamp)
{
	/* Bgnd task context */
	lOsBgnd.wakeup_time = tstamp * OS_TICKS_PER_US;
}

tOsTimestamp Os_BgndGetCurrentTime(void)
{
	/* Bgnd task context */
	uint32 last_time;
	uint32 crt_time;
	tOsTimestamp total_time;

	/* get the OS time values in a critical section */
	Os_Disable_RT_Interrupt();
	crt_time = HALOsSys_GetCurrentCounter();
	last_time = lOsRt.tstamp_last;
	total_time = lOsRt.time_ctr;
	Os_Enable_RT_Interrupt();

    if (last_time > crt_time)
    {
    	total_time += (uint64)(OSSYS_TIMER_MAX_CYCLES - last_time + crt_time);
    }
    else
    {
    	total_time += (uint64)(crt_time - last_time);
    }

    return (total_time / OS_TICKS_PER_US);
}


/*--------------------------------------------------
 *   		   Exported Function
 *--------------------------------------------------*/

void Os_ShedulerTimedTaskEntry(void)
{
	/*
	 * gHALOsSys_CounterValue - is the counter value when the timer interrupt is produced
	 * gHALOsSys_CounterNext  - is the next trigger event for the time counter
	 */

	uint32 count;
	uint32 next_timestamp;
	uint32 time_diff;

	/* increment the Os time counter
	 * Note: do this first because cyclic tasks rely on this value to get elapsed time */
	count = lOsRt.tstamp_last;
    if (count > gHALOsSys_CounterValue)
    {
    	lOsRt.time_ctr += (uint64)(OSSYS_TIMER_MAX_CYCLES - count + gHALOsSys_CounterValue);
    }
    else
    {
    	lOsRt.time_ctr += (uint64)(gHALOsSys_CounterValue - count);
    }
    lOsRt.tstamp_last = gHALOsSys_CounterValue;


	/*
	 * -----  execute the task at the set index ----
	 */
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
    lOsRtWake_TaskHasRun = ~0;
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

	lOsRtWake_TaskHasRun = 0;
	lOsRtWake_TaskEvent = 0;
	lOsRtWake_Timestamp = 0;
	lOsRt.tstamp_current = gOsRt_TaskList[0].timeStamp;
	/* set up the first time event to trigger the first task */
	gHALOsSys_CounterNext = gOsRt_TaskList[0].timeStamp;
}

