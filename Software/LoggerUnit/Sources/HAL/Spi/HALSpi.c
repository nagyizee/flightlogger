/**
 *
 * SPI driver first version
 * Static configuration
 * 1 Channel only
 * Fixed Chip select
 */

/*--------------------------------------------------
 * 		    		Includes
 *--------------------------------------------------*/
#include "HALSpi.h"

/*--------------------------------------------------
 * 		    		Defines
 *--------------------------------------------------*/

#define HALSPI_MAXBUFFERSIZE  260u  /* FLS write/read cmd + 3 byte address + 256 data bytes */

/*--------------------------------------------------
 * 	       Local Variables and prototypes
 *--------------------------------------------------*/

uint8 HALSPI_DummyDataBuffer[HALSPI_MAXBUFFERSIZE];

/*--------------------------------------------------
 *   		   Interface Functions
 *--------------------------------------------------*/

/* Device handling functions */

void HALSPI_Init(void)
{
    /* primitive hardcoded implementation */

    /* enable peripheral and set up baudrate, output pins, etc. */

    NRF_SPIM1->ENABLE = SPIM_ENABLE_ENABLE_Enabled;



};

tSpiStatus HALSPI_Status(tSpiChannelType ch)
{
    return SPI_UNINITIALIZED;
};

TSpiStatus HALSPI_StartTransfer(tSpiChannelType ch)
{
    return SPI_READY;
};

/* Chip select handling functions */

tSpiStatus HALSPI_SetCS(tSpiChannelType ch)
{
    /* Dummy LED toggling */
    PORT_PIN_LED_BLE_ON();
    return SPI_READY;
};

void HALSPI_ReleaseCS(tSpiChannelType ch)
{
    /* Dummy LED toggling */
    PORT_PIN_LED_BLE_OFF();
};

/* Data handling functions */

void HALSPI_TxData(tSpiChannelType ch, uint16 cnt , uint8 *buf)
{ 
    uint16 i;
    
    /* Save dummy data */
    for(i=0; i<cnt; i++) HALSPI_DummyDataBuffer[i] = buf[i];

};

void HALSPI_RxData(tSpiChannelType ch, uint16 cnt , uint8 *buf)
{
    uint16 i;
    
    /* Send back dummy data */
    for(i=0; i<cnt; i++) buf[i] = HALSPI_DummyDataBuffer[i];
};

/*--------------------------------------------------
 *   		   Local Functions
 *--------------------------------------------------*/

