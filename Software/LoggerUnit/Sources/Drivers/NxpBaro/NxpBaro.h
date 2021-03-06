#ifndef _NXPBARO_H
#define _NXPBARO_H
/**
 *
 *  NXP mpl3115A2 barometer driver
 *
 */

/*--------------------------------------------------
 *                              Includes
 *--------------------------------------------------*/

#include "base.h"

/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/

#define NXPBARO_ACQMASK_PRESSURE     (0x01u)
#define NXPBARO_ACQMASK_ALTITUDE     (0x02u)
#define NXPBARO_ACQMASK_TEMP         (0x04u)

#define NXPBARO_TEMP_OFFSET          (100u)                 /* integer offset for temperature ( value 0 means -100*C) */
#define NXPBARO_ALT_OFFSET           (1000u)                /* integer offset for altitude    ( value 0 means -1000m) */
#define NXPBARO_ERROR_CODE           (0xFFFFFFFFu)

/* operating configuration parameters */
#define NXPBARO_CFG_ERROR_RETRY             (3)             /* 3 retrials in case of comm. error */
#define NXPBARO_CFG_TIMEROUT_CTR           (100)            /* in cyclic call ticks - maximum timeout for waiting the sensor interrupt */

/* driver states */
typedef enum
{
    NXPBARO_ST_UNINITTED  = 0u,        /* driver and device not initialized yet */
    NXPBARO_ST_INITIALIZING,           /* driver and device under initialization phase (from uninit or from low power) */
    NXPBARO_ST_IDLE,                   /* device is in idle state - no measurement result available */
    NXPBARO_ST_READY,                  /* device is in idle state - measurement result is available. Mask is returned by _GetStatus function */
    NXPBARO_ST_BUSY,                   /* device is busy with measurement */
    NXPBARO_ST_LOW_PWR,                /* device is in low power mode */
    NXPBARO_ST_ERROR                   /* device error - can not communicate with the device */
} tNxpBaroStatus;

/* measurement selector */
typedef enum
{
    NXPBARO_MS_PRESSURE = NXPBARO_ACQMASK_PRESSURE,
    NXPBARO_MS_ALTITUDE = NXPBARO_ACQMASK_ALTITUDE,
    NXPBARO_MS_TEMP = NXPBARO_ACQMASK_TEMP,
} tNxpBaroMeasurementSelector;

/*--------------------------------------------------
                                Defines
 *--------------------------------------------------*/



/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/

/* Initialize the barometer driver - device will put in low power mode by default */
void NXPBaro_Init(void);

/* Main function call for the barometer driver */
void NXPBaro_MainFunction(void);

/* Get the status of the barometer.
 * acq_mask will return the mask with the finished measurements in case of NXPBARO_READY state if the provided pointer is non_NULL
 * acq_mask flag is cleared when the selected result is read */
tNxpBaroStatus NXPBaro_GetStatus(uint32 *acq_mask);

/* Put the barometer device in low power mode. Any ongoing measurement or operation is cancelled
 * Note - function is asynchronous */
void NXPBaro_Sleep(void);

/* Acquire request for one or more measurement parameters.
 * If device was in low power mode then it will be woken up
 * For device wakeup only - call the function with no maks - this will not trigger measurement,
 * only the device wakeup. */
tResult NXPBaro_Acquire(uint32 mask);

/* Return the selected measurement result.
 * In case of failure or if result is not ready - NXPBARO_ERROR_CODE is returned
 * When a measurement result is read, it's acquire mask flag is cleared
 * Format of returned data:
 * - altitude:    FPu16.16 in meters with offset (defined above)
 * - pressure:    FPu24.8  in Pa
 * - temperature: FPu16.16 in *C with offset (defined above)
 **/
uint32 NXPBaro_GetResult(tNxpBaroMeasurementSelector select);

#endif

