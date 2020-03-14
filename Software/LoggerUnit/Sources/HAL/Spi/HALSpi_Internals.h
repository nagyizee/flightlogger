#ifndef HAL_SPI_HALSPI_INTERNALS_H_
#define HAL_SPI_HALSPI_INTERNALS_H_

/*--------------------------------------------------
 *                              Includes
 *--------------------------------------------------*/

#include "HALSpi.h"

/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/

#define SPI_CHANNEL_FREE        (0xFFu)

typedef struct
{
    uint8*  tx_buff;
    uint8*  rx_buff;
    uint16  tx_size;
    uint16  rx_size;

    uint16  tx_remaining;
    uint16  rx_remaining;
} tSpiChannelParams;


typedef struct
{
    bool            busy;                     /* i2c peripheral is busy */
    tSpiChannelType op_ch;                    /* channel in operation */

    tSpiChannelParams ch[HALSPI_CH_NR];       /* operational parameters for each channel */

} tSpiInternals;


typedef struct
{
    uint8           cs_pin;                   /* pin used for chipselect */
} tSpiChannelConfig;


/*--------------------------------------------------
                                Defines
 *--------------------------------------------------*/


/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/


#endif /* HAL_SPI_HALSPI_INTERNALS_H_ */
