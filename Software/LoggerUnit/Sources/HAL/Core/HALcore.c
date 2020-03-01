#include "base.h"
#include "nrf.h"

void HALCore_Init(void)
{
    /* set vector table to the flash start */
    SCB->VTOR = 0x00000000;

    /* start external oscillator and switch it as HF clock source */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1U;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0)
    {
    }
}
