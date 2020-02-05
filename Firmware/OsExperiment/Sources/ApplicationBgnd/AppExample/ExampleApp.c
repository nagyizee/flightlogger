#include "base.h"
#include "HALport.h"

void ExampleApp_Main(void)
{
	volatile uint32 count;

	count = 1000000;
   PORT_PIN_BGNDAPP_ON();
	while (count)
	{
		count--;
	}
	PORT_PIN_BGNDAPP_OFF();
}
