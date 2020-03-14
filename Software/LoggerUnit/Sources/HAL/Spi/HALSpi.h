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
#include "HALSpi_Cfg.h"

/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/

typedef uint8 tSpiChannelType;

typedef enum 
{ 
    SPI_UNINITIALIZED = 0u,         /* peripheral is shut down or in low power */
    SPI_IDLE,                       /* peripheral is in idle state, no chipselect */
    SPI_READY,                      /* chipselect was issued for the current channel, but no operation on it. The rest of channels will report SPI_BUSY */
    SPI_BUSY                        /* channel is transmitting/receiving, or an other channel is selected and/or operational. */
} tSpiStatus;
/*--------------------------------------------------
                                Defines
 *--------------------------------------------------*/


/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/

/* Device handling functions */

void HALSPI_Init(void);
tSpiStatus HALSPI_Status(tSpiChannelType ch);
tResult HALSPI_StartTransfer(tSpiChannelType ch);

/* Chip select handling functions */

/* Select an SPI device by it's channel.
 * The routine allocates operations for this channel - any operation
 * on other channels will result in invalid or busy result.
 */
tResult HALSPI_SetCS(tSpiChannelType ch);
/* Deselect an SPI device.
 * It will release the channel operations also.
 * - If called ch is busy, then it will stop the operation on that channel (tx/rx is stopped, buffers remain incompletely filled)
 * - If called on a non-chipselected channel then it does nothing
 */
void HALSPI_ReleaseCS(tSpiChannelType ch);

/* Data handling functions.
 * - These functions can be called on any channel, only the current channel must not be in BUSY state.
 * - By setting cnt to 0 or buf to NULL, the operation will be canceled for a following StartTransfer */
tResult HALSPI_TxData(tSpiChannelType ch, uint16 cnt , uint8 *buf);
tResult HALSPI_RxData(tSpiChannelType ch, uint16 cnt , uint8 *buf);

#endif





