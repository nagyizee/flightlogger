#ifndef	_HALSPI_H
#define _HALSPI_H
/**
 *
 * SPI driver first version
 * Static configuration
 * 1 Channel only
 * Fixed Chip select
 */

/*--------------------------------------------------
 *                              Includes
 *--------------------------------------------------*/

#include "base.h"
#include "HALport.h"

/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/

typedef uint32 tSpiChannelType;

typedef enum 
{ 
    SPI_UNINITIALIZED = 0u,
    SPI_BUSY,
    SPI_READY,
} TSpiStatus;
/*--------------------------------------------------
                                Defines
 *--------------------------------------------------*/


/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/

/* Device handling functions */

void HALSPI_Init(void);
tSpiStatus HALSPI_Status(tSpiChannelType ch);
tSpiStatus HALSPI_StartTransfer(tSpiChannelType ch);

/* Chip select handling functions */

tSpiStatus HALSPI_SetCS(tSpiChannelType ch);
void HALSPI_ReleaseCS(tSpiChannelType ch);

/* Data handling functions */

void HALSPI_TxData(tSpiChannelType ch, uint16 cnt , uint8 *buf);
void HALSPI_RxData(tSpiChannelType ch, uint16 cnt , uint8 *buf);

#endif





