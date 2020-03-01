extern void ISRHandler_OsTimer(void);


void TIMER0_IRQHandler(void)
{
	ISRHandler_OsTimer();
}
