/*
 * HALI2c_Internals.h
 *
 */

#ifndef HAL_I2C_HALI2C_INTERNALS_H_
#define HAL_I2C_HALI2C_INTERNALS_H_

/*--------------------------------------------------
 *                              Includes
 *--------------------------------------------------*/

#include "HALI2c.h"

/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/

#define I2C_BAUDRATE_100        (0x01980000u)
#define I2C_BAUDRATE_250        (0x04000000u)
#define I2C_BAUDRATE_400        (0x06400000u)

typedef struct
{
    tI2CStatus  ch_status;


} tI2cInternals;


typedef struct
{
    uint8  dev_addr;        /* slave device address */


} tI2cChannelConfig;


/*--------------------------------------------------
                                Defines
 *--------------------------------------------------*/


/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/



#endif /* HAL_I2C_HALI2C_INTERNALS_H_ */
