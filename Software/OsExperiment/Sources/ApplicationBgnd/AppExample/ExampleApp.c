#include "base.h"
#include "Os.h"
#include "HALport.h"

static uint32 bulk_runs = 0;

void local_LongRoutine(uint32 time);

void ExampleApp_Main(uint32 reason)
{
	PORT_PIN_BGNDAPP_ON();
	bulk_runs++;
	bulk_runs &= 0x0F;
	/* set up run mode */
	if ((bulk_runs == 3) )
	{
		Os_BgndRunMode(eOsBgnd_periodRt);
	}
	else if (bulk_runs == 9)
	{
		Os_BgndRunMode(eOsBgnd_RtAppCtrl);
	}
	else if (bulk_runs == 14)
	{
		tOsTimestamp crt_time;
		crt_time = Os_BgndGetCurrentTime();
		Os_BgndRunMode(eOsBgnd_timestamp);
		Os_BgndRunTimestamp(crt_time + 20000);
	}
	else if (bulk_runs == 15)
	{
		Os_BgndRunMode(eOsBgnd_bulk);
	}

	if (bulk_runs < 4)
	{
		/* bulk */
		local_LongRoutine(2);
	}
	else if (bulk_runs < 10)
	{
		/* rt periods */
	}
	else if (bulk_runs < 15)
	{
		/* rt app ctrl */
		reason += 2;
		while (reason)
		{
			PORT_PIN_BGNDAPP_OFF();
			reason--;
			asm("nop");
			PORT_PIN_BGNDAPP_ON();
		}
		local_LongRoutine(1);
	}
	else
	{
		/* timestamp */
		local_LongRoutine(8);
	}
	PORT_PIN_BGNDAPP_OFF();
}


void local_LongRoutine(uint32 time)
{
	volatile uint32 count;

	count = 500 * time;
	while (count)
	{
		count--;
	}
}

