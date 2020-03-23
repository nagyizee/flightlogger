#ifndef _HALI2C_H
#define _HALI2C_H
/**
 *
 * I2C driver first version
 * Static configuration
 *
 */

/*--------------------------------------------------
 *                              Includes
 *--------------------------------------------------*/

#include "base.h"
#include "HALI2C_Cfg.h"

/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/

typedef enum
{
    I2C_IDLE  = 0u,     /* I2C channel is idle and ready to be used */
    I2C_BUSY_CH,        /* I2C channel is busy sending or receiving data */
    I2C_BUSY_OTHER,     /* I2C device is busy but current channel terminated the operation (data is sent or read result is in, but can not give new command) */
    I2C_ERR_NAK,        /* I2C channel received unexpected NAK from the addressed slave, or the slave device is refusing receiving more data */
    I2C_ERR_ARB,        /* I2C arbitration error on the bus used by the channel */
    I2C_ERR_TIMEOUT,    /* I2C channel in blocked state (long clock stretch from the addressed slave) - reinit is needed */
    I2C_ERR_INVALID,    /* Status function called on invalid channel */
} tI2CStatus;

typedef uint8   tI2CChannelType;

/*--------------------------------------------------
                                Defines
 *--------------------------------------------------*/



/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/

/* Init I2C hardware and driver internals, or do a reinit in case of interface lock-up */
void HALI2C_Init(void);

/* I2C main cyclic function */
void HALI2C_MainFunction(void);

/* Get I2C channel status.
 * Note: The function call does an internal polling loop.
 *       can be used to operate I2C in synchronous way by polling this function */
tI2CStatus HALI2C_GetStatus(tI2CChannelType ch);

/* Write a register to a slave device on it's assigned channel.
 * Asynchronous routine - check data availability and keep the buffer available till channel becomes idle */
tResult HALI2C_Write(tI2CChannelType ch, const uint8 *buffer, uint32 size);

/* Read data from a slave device on it's assigned channel.
 * Asynchronous routine - check data availability and keep the buffer available till channel becomes idle */
tResult HALI2C_Read(tI2CChannelType ch, uint8 *buffer, uint32 size);

/* Read data from a slave device on it's assigned channel from a given register.
 * Bus turn around is done automatically in the driver.
 * Asynchronous routine - check data availability and keep the buffer available till channel becomes idle */
tResult HALI2C_ReadRegister(tI2CChannelType ch, uint8 dev_register, uint8 *buffer, uint32 size);

#endif





