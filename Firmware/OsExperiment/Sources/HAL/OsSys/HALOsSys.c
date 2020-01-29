#include "HALOsSys.h"


#define OSSYS_TIMER_PRESCALER		(8 - 1)		/* divide sys.clock by 8 ->  3MHz = 3 ticks per us

												   timer counter full range is enough for 21.8ms */

/* Function definition implemented in the Os, called in OsSys */
extern void Os_ShedulerTimedTaskEntry(void);

/* Exported variables for the Os */
volatile uint32 gHALOsSys_CounterValue;
volatile uint32 gHALOsSys_CounterNext;



void HALOsSys_Init(void)
{
	/* Initialize timer for interrupt generator */
	/* enable timer RCC clock */
	RCC->APB1ENR |= RCC_APB2ENR_TIM17EN_Msk;
	/* configure the timer - nothing special, enable compare 1 interrupt */
	TIM17->CR2 = 0x0000;
	TIM17->DIER = TIM_DIER_CC1IE;
	/* reset stuff */
	TIM17->CNT = 0x0000;
	TIM17->SR = 0x0000;
	/* set prescaler and full range for the counter */
	TIM17->PSC = OSSYS_TIMER_PRESCALER;
	TIM17->ARR = 0xFFFF;
	/* generate the first interrupt right at the beginning */
	TIM17->CCR1 = 0x0000;

	/* freeze timer in debug */
	DBGMCU->CR = DBGMCU_CR_DBG_TIM17_STOP;

	/* set up NVIC */

	/* start the timer */
	TIM17->CR1 = TIM_CR1_CEN;
}



void ISRHandler_OsTimer(void)
{
	gHALOsSys_CounterValue = (uint32)TIM17->CNT;
	Os_ShedulerTimedTaskEntry();
	TIM17->SR &= (uint16)~TIM_SR_CC1IF;
	TIM17->CCR1 = (uint16)gHALOsSys_CounterNext;
}
