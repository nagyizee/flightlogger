#include "base.h"
#include "stm32f100xb.h"


void HALCore_Init(void)
{
	/* init vector table */
	SCB->VTOR = 0x00000000;

	/* Init system clock with external oscillator */
	/* start up external oscillator */
	RCC->CR |= RCC_CR_HSEON_Msk;
	while (RCC->CR & RCC_CR_HSERDY_Msk == 0);
    /* set up PLL for 24MHz - same clock for AHB / APB1 / APB2 */
	RCC->CFGR = RCC_CFGR_PLLMULL3 | RCC_CFGR_PLLSRC_Msk;
	RCC->CR |= RCC_CR_PLLON_Msk;
	while (RCC->CR & RCC_CR_PLLRDY_Msk == 0);
	/* switch system clock source to PLL */
	RCC->CFGR |= 0x01;
	RCC->CR &= ~RCC_CR_HSION_Msk;
}
