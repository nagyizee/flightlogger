#if 1       /* NRF52832 */

#include "base.h"
#include "nrf.h"
#include "HALport.h"

void ISRHandler_Int1(void)
{
	uint32 cntr;
	PORT_PIN_INTR1_ON();
	NRF_TIMER1->EVENTS_COMPARE[0] = 0;

	for (cntr = 0; cntr < 75; cntr++)
	{
		asm("nop");
		asm("nop");
	}

	PORT_PIN_INTR1_OFF();
}

void ISRHandler_Int2(void)
{
	uint32 cntr;
	PORT_PIN_INTR2_ON();
    NRF_TIMER2->EVENTS_COMPARE[0] = 0;

	for (cntr = 0; cntr < 100; cntr++)
	{
		asm("nop");
		asm("nop");
	}

	PORT_PIN_INTR2_OFF();
}

void ExampleDrv_Init(void)
{
	/* initialize two timers */
	/* set up for interrupt generator Tim6: 1.6ms,   Tim7: 1.8ms  */
    NRF_TIMER1->TASKS_CLEAR = 1;
    NRF_TIMER1->MODE = TIMER_MODE_MODE_Timer;
    NRF_TIMER1->BITMODE = TIMER_BITMODE_BITMODE_16Bit;
    NRF_TIMER1->PRESCALER = 3;
    NRF_TIMER1->CC[0] = 1;
    NRF_TIMER1->CC[1] = 3333;
    NRF_TIMER1->SHORTS = TIMER_SHORTS_COMPARE1_CLEAR_Msk;
    NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE0_Msk;

    NRF_TIMER2->TASKS_CLEAR = 1;
    NRF_TIMER2->MODE = TIMER_MODE_MODE_Timer;
    NRF_TIMER2->BITMODE = TIMER_BITMODE_BITMODE_16Bit;
    NRF_TIMER2->PRESCALER = 3;
    NRF_TIMER2->CC[0] = 1;
    NRF_TIMER2->CC[1] = 3777;
    NRF_TIMER2->SHORTS = TIMER_SHORTS_COMPARE1_CLEAR_Msk;
    NRF_TIMER2->INTENSET = TIMER_INTENSET_COMPARE0_Msk;

	/* set up NVIC */
	NVIC_SetPriority(TIMER1_IRQn, 0);
	NVIC_EnableIRQ(TIMER1_IRQn);
	NVIC_SetPriority(TIMER2_IRQn, 0);
	NVIC_EnableIRQ(TIMER2_IRQn);

	/* enable timers */
    NRF_TIMER1->TASKS_START = 1;
    NRF_TIMER2->TASKS_START = 1;
}

#elif 0     /* STM32F100 */

#include "base.h"
#include "stm32f100xb.h"
#include "HALport.h"

void ISRHandler_Int1(void)
{
    uint32 cntr;
    PORT_PIN_INTR1_ON();
    TIM6->SR = 0x0000;

    for (cntr = 0; cntr < 75; cntr++)
    {
        asm("nop");
        asm("nop");
    }

    PORT_PIN_INTR1_OFF();
}

void ISRHandler_Int2(void)
{
    uint32 cntr;
    PORT_PIN_INTR2_ON();
    TIM7->SR = 0x0000;

    for (cntr = 0; cntr < 100; cntr++)
    {
        asm("nop");
        asm("nop");
    }

    PORT_PIN_INTR2_OFF();
}

void ExampleDrv_Init(void)
{
    /* initialize two timers */
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN | RCC_APB1ENR_TIM7EN;
    /* set up for interrupt generator Tim6: 1.6ms,   Tim7: 1.8ms  */
    TIM6->CR2 = 0x0000;
    TIM7->CR2 = 0x0000;
    TIM6->CNT = 0x0000;
    TIM7->CNT = 0x0000;
    TIM6->DIER = 0x0001;
    TIM7->DIER = 0x0001;
    TIM6->PSC = 0x0000;
    TIM7->PSC = 0x0000;
    TIM6->ARR = 38400;
    TIM7->ARR = 39600;

    /* set up NVIC */
    NVIC_SetPriority(TIM6_DAC_IRQn, 0);
    NVIC_EnableIRQ(TIM6_DAC_IRQn);
    NVIC_SetPriority(TIM7_IRQn, 0);
    NVIC_EnableIRQ(TIM7_IRQn);

    /* enable timers */
    TIM6->CR1 = TIM_CR1_CEN;
    TIM7->CR1 = TIM_CR1_CEN;
}

#endif
