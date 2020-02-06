#include "base.h"
#include "Os.h"
#include "HALport.h"

static uint32 bulk_runs = 0;

void local_LongRoutine(void);

void ExampleApp_Main(uint32 reason)
{
	PORT_PIN_BGNDAPP_ON();
	bulk_runs++;
	bulk_runs &= 0x0F;
	if ((bulk_runs & 0x0C) == 0x0C)
	{
		local_LongRoutine();
		Os_BgndRunMode(eOsBgnd_bulk);
	}
	else
	{
		Os_BgndRunMode(eOsBgnd_periodRt);
	}
	PORT_PIN_BGNDAPP_OFF();
}


void local_LongRoutine(void)
{
	volatile uint32 count;

	count = 1000;
	while (count)
	{
		count--;
	}
}

