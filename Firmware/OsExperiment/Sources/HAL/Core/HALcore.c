#include "base.h"
#include "stm32f100xb.h"


void HALCore_Init(void)
{
	/* init vector table */
	SCB->VTOR = 0x00000000;

	/* Init system clock with external oscillator */
	/* start up external oscillator */
	RCC->CR |= RCC_CR_HSEON;
	while (RCC->CR & RCC_CR_HSERDY == 0);
    /* set up PLL for 24MHz - same clock for AHB / APB1 / APB2 */
	RCC->CFGR = RCC_CFGR_PLLMULL3 | RCC_CFGR_PLLSRC;
	RCC->CR |= RCC_CR_PLLON;
	while (RCC->CR & RCC_CR_PLLRDY == 0);
	/* switch system clock source to PLL */
	RCC->CFGR |= 0x02;
	RCC->CR &= ~RCC_CR_HSION;
}
