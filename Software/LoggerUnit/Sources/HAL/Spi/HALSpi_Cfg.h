#ifndef HAL_SPI_HALSPI_CFG_H_
#define HAL_SPI_HALSPI_CFG_H_

/*--------------------------------------------------
 *                              Includes
 *--------------------------------------------------*/

#include "HALPort.h"

/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/


/*--------------------------------------------------
                                Defines
 *--------------------------------------------------*/

/* number of channels */
#define HALSPI_CH_NR                   (1u)
/* channel indexes */
#define HALSPI_CHANNEL_CYPFLASH        (0u)

/* chipselect pins for the various channels */
#define HALSPI_CHIPSEL_CYPFLASH        (PIN_FLS_CS)

/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/

#endif /* HAL_SPI_HALSPI_CFG_H_ */
