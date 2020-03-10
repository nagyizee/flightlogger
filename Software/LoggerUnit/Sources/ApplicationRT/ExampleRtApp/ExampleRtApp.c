#include "ExampleRtApp.h"
#include "HALport.h"
#include "Os.h"

//TODO: remove this
#include "HALI2c.h"


uint8 buff[16];

void ExampleRtApp_Main(uint32 taskIdx)
{
	volatile uint32 count;

	for (count = 0; count < taskIdx; count++)
	{
		PORT_PIN_LED_ON_ON();
		asm("nop");
		asm("nop");
		PORT_PIN_LED_ON_OFF();
	}

	if (taskIdx)
	{
		count = 200;
	}
	else
	{
		count = 50;
	}

	PORT_PIN_LED_ON_ON();
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

	PORT_PIN_LED_ON_OFF();

static bool i2ctest = false;
if (i2ctest)
{
    volatile uint32 ctr = 0;
    buff[0] = 0x26;
    buff[1] = 0xB8;
    buff[2] = 0xFF;

    HALI2C_Write(HALI2C_CHANNEL_BAROMETER, buff, 3);

    while (HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER) == I2C_BUSY)
    {
        ctr++;
    }

    i2ctest = 0;
}



}
