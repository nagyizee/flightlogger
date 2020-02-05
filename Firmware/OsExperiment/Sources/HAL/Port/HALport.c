#include "base.h"
#include "HALport.h"
#include "stm32f100xb.h"


void HALPort_Init(void)
{
	/* enable peripheral clocks for the GPIO ports */
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

	/* set up ouptut registers for portB*/
	GPIOB->ODR = 0x00000000;
	GPIOB->CRH = GPIO_CRH_MODE10_0 | GPIO_CRH_MODE12_0 | GPIO_CRH_MODE14_0 | GPIO_CRH_MODE15_0;
}
