#include "ExampleRtApp.h"
#include "HALport.h"


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
	PORT_PIN_RTAPP_OFF();
}
