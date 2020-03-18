#include "HALOsSys.h"
#include "nrf.h"


#define OSSYS_TIMER_PRESCALER		(3)		    /* divide 16MHz tim.clock by 2^3 ->  2MHz = 2 ticks per us

												   timer counter full range is enough for 32.7ms */

/* Function definition implemented in the Os, called in OsSys */
extern void Os_ShedulerTimedTaskEntry(void);

/* Exported variables for the Os. Note: CounterNext may be set up before HALOsSys_Init by the Os itself */
volatile uint32 gHALOsSys_CounterValue = 0x0000;
volatile uint32 gHALOsSys_CounterNext = 0x0000;



void HALOsSys_Init(void)
{
    /* Initialize timer for interrupt generator */
	NRF_TIMER0->TASKS_CLEAR = 1;
	NRF_TIMER0->MODE = TIMER_MODE_MODE_Timer;
	NRF_TIMER0->BITMODE = TIMER_BITMODE_BITMODE_16Bit;
	NRF_TIMER0->PRESCALER = OSSYS_TIMER_PRESCALER;
	NRF_TIMER0->CC[0] = gHALOsSys_CounterNext;
    NRF_TIMER0->EVENTS_COMPARE[0] = 0;

	/* set up NVIC */
	NVIC_SetPriority(TIMER0_IRQn, 1);
	NVIC_EnableIRQ(TIMER0_IRQn);

    NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
	/* start the timer */
	NRF_TIMER0->TASKS_START = 1;
}

void ISRHandler_OsTimer(void)
{
    NRF_TIMER0->TASKS_CAPTURE[1] = 1;
	gHALOsSys_CounterValue = NRF_TIMER0->CC[1];
	Os_ShedulerTimedTaskEntry();
	NRF_TIMER0->CC[0] = (uint16)gHALOsSys_CounterNext;
	NRF_TIMER0->EVENTS_COMPARE[0] = 0;
}
