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
    bool            busy;                     /* i2c peripheral is busy */
    tI2CChannelType op_ch;                    /* channel in operation */
    tI2CStatus      ch_status[HALI2C_CH_NR];  /* memorize individual statuses for each channel to hold error state even after op. successfully the other channels */
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
