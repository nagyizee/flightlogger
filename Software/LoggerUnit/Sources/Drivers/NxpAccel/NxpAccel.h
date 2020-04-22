#ifndef _NXPACCEL_H
#define _NXPACCEL_H
/**
 *
 *  NXP MMA8452 3x axis accelerometer driver
 *
 */

/*--------------------------------------------------
 *                              Includes
 *--------------------------------------------------*/

#include "base.h"

/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/

#define NXPACCEL_ACQ_SINGLE           (0x01u)
#define NXPACCEL_ACQ_CONTINUOUS       (0x02u)

/* operating configuration parameters */
#define NXPACCEL_CFG_ERROR_RETRY      (3)             /* 3 retrials in case of comm. error */
#define NXPACCEL_CFG_TIMEROUT_CTR     (100)           /* in cyclic call ticks - maximum timeout for waiting the sensor interrupt */

/* driver states */
typedef enum
{
    NXPACCEL_ST_UNINITTED  = 0u,        /* driver and device not initialized yet */
    NXPACCEL_ST_INITIALIZING,           /* driver and device under initialization phase */
    NXPACCEL_ST_IDLE,                   /* device is in low power idle state - no measurement is ongoing and no result available */
    NXPACCEL_ST_READY,                  /* measurement result is available */
    NXPACCEL_ST_BUSY,                   /* device is busy with measurement */
    NXPACCEL_ST_HW_ERROR,               /* device communication error */
    NXPACCEL_ST_SENS_ERROR              /* selftest failed - sensor is communicating but measurements are not guaranteed */
} tNxpAccelStatus;

/* accelerometer result structure */
typedef struct
{
    int16  X;
    int16  Y;
    int16  Z;
} tNxpAccelResult;

/*--------------------------------------------------
                                Defines
 *--------------------------------------------------*/



/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/

/* Initialize the accelerometer driver - device will put in low power mode by default */
void NXPAccel_Init(void);

/* Main cyclic function call for the accelerometer driver */
void NXPAccel_MainFunction(void);

/* Get the status of the accelerometer. */
tNxpAccelStatus NXPAccel_GetStatus();

/* Accelerometer is in low power as default, single measurement will reenter in this state.
 * If continuous acquisition is done the it will be halted by this call.
 * Any ongoing measurement or operation is cancelled
 * Note - function is asynchronous */
void NXPAccel_Sleep(void);

/* Acquire request.
 * I will trigger a single measurement or continuous measurement in fn. of aqc_type.
 * see values of NXPACCEL_ACQ_ defines. */
tResult NXPAccel_Acquire(uint32 acq_type);

/* Return a measurement set.
 * If no result is available it returns RES_INVALID
 * Results are buffered in an internal fifo,
 * poll this routine for results till it returns RES_INVALID to empty the fifo.
 * Note that NXPAccel_Sleep() will clear the fifo.
 * */
tResult NXPAccel_GetResult(tNxpAccelResult *result);

#endif

