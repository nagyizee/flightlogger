
extern void ISRHandler_OsTimer(void);


void TIM1_TRG_COM_TIM17_IRQHandler(void)
{
	ISRHandler_OsTimer();
}






