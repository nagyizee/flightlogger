
extern void ISRHandler_OsTimer(void);
extern void ISRHandler_Int1(void);
extern void ISRHandler_Int2(void);

/* scheduled event ISR for the OS */
#if 1       /* NRF52832 */

void TIMER0_IRQHandler(void)
{
	ISRHandler_OsTimer();
}

void TIMER1_IRQHandler(void)
{
	ISRHandler_Int1();
}

void TIMER2_IRQHandler(void)
{
	ISRHandler_Int2();
}


#elif 0     /* STM32F100 */

void TIM1_TRG_COM_TIM17_IRQHandler(void)
{
    ISRHandler_OsTimer();
}

void TIM6_DAC_IRQHandler(void)
{
    ISRHandler_Int1();
}

void TIM7_IRQHandler(void)
{
    ISRHandler_Int2();
}

#endif

