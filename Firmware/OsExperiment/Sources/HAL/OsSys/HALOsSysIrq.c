
extern void ISRHandler_OsTimer(void);
extern void ISRHandler_Int1(void);
extern void ISRHandler_Int2(void);

/* scheduled event ISR for the OS */
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


