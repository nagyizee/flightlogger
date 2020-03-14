extern void ISRHandler_OsTimer(void);
extern void ISRHandler_Spi(void);

void TIMER0_IRQHandler(void)
{
	ISRHandler_OsTimer();
}

void SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler(void)
{
    ISRHandler_Spi();
}
