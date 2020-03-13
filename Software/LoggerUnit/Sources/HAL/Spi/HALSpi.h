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

typedef uint32 TSpiChannelType;

typedef enum 
{ 
    SPI_UNINITIALIZED = 0u,
    SPI_BUSY,
    SPI_READY,
} TSpiStatus;

/*--------------------------------------------------
                                Defines
 *--------------------------------------------------*/

#define HALSPI_MAXBUFFERSIZE  260u  /* FLS write/read cmd + 3 byte address + 256 data bytes */

/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/

/* Device handling functions */

void HALSPI_Init(void);
TSpiStatus HALSPI_Status(TSpiChannelType ch);
TSpiStatus HALSPI_StartTransfer(TSpiChannelType ch);

/* Chip select handling functions */

TSpiStatus HALSPI_SetCS(TSpiChannelType ch);
void HALSPI_ReleaseCS(TSpiChannelType ch);

/* Data handling functions */

void HALSPI_TxData(TSpiChannelType ch, uint16 cnt , uint8 *buf);
void HALSPI_RxData(TSpiChannelType ch, uint16 cnt , uint8 *buf);

#endif





