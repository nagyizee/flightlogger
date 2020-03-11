#include "ExampleRtApp.h"
#include "HALport.h"
#include "Os.h"

//TODO: remove this
#include "HALI2c.h"


uint8 buff[16];

static uint32 i2ctest = 0;
static tI2CChannelType ch = HALI2C_CHANNEL_BAROMETER;
static uint32 tx_size = 1;
static uint32 rx_size = 1;
static uint8 reg;

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

if (i2ctest)
{
    volatile uint32 ctr = 0;

    if (i2ctest == 1)
    {
        HALI2C_Write(ch, buff, tx_size);
    }
    else if (i2ctest == 2)
    {
        HALI2C_Read(ch, buff, rx_size);
    }
    else
    {
        HALI2C_ReadRegister(ch, reg, buff, rx_size);
    }

    while (HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER) == I2C_BUSY)
    {
        ctr++;
    }

    i2ctest = 0;
}



}
