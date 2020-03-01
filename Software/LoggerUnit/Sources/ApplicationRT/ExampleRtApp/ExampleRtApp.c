#include "ExampleRtApp.h"
#include "HALport.h"
#include "Os.h"


void ExampleRtApp_Main(uint32 taskIdx)
{
	volatile uint32 count;

	for (count = 0; count < taskIdx; count++)
	{
		PORT_PIN_RTAPP_ON();
		asm("nop");
		asm("nop");
		PORT_PIN_RTAPP_OFF();
	}

	if (taskIdx)
	{
		count = 200;
	}
	else
	{
		count = 50;
	}

	PORT_PIN_RTAPP_ON();
	while (count)
	{
		count--;
	}

	if (taskIdx == 2)
	{
		Os_RtWakeUpBgndTask(2);
	}
	else if (taskIdx == 4)
	{
		Os_RtWakeUpBgndTask(4);
	}

	PORT_PIN_RTAPP_OFF();
}
