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
};

TSpiStatus HALSPI_Status(TSpiChannelType ch)
{
  return SPI_UNINITIALIZED;
};

TSpiStatus HALSPI_StartTransfer(TSpiChannelType ch)
{
  return SPI_READY;
};

/* Chip select handling functions */

TSpiStatus HALSPI_SetCS(TSpiChannelType ch)
{
  /* Dummy LED toggling */
  PORT_PIN_LED_BLE_ON();
  return SPI_READY;
};

void HALSPI_ReleaseCS(TSpiChannelType ch)
{
  /* Dummy LED toggling */
  PORT_PIN_LED_BLE_OFF();
};

/* Data handling functions */

void HALSPI_TxData(TSpiChannelType ch, uint16 cnt , uint8 *buf)
{ 
  uint16 i;
  
  /* Save dummy data */
  for(i=0; i<cnt; i++) HALSPI_DummyDataBuffer[i] = buf[i];

};

void HALSPI_RxData(TSpiChannelType ch, uint16 cnt , uint8 *buf)
{
  uint16 i;
  
  /* Send back dummy data */
  for(i=0; i<cnt; i++) buf[i] = HALSPI_DummyDataBuffer[i];
};

/*--------------------------------------------------
 *   		   Local Functions
 *--------------------------------------------------*/

