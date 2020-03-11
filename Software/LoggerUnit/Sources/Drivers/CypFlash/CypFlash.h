#ifndef _CYPFLASH_H
#define _CYPFLASH_H
/**
 *
 *  Cypress S25FL064 External flash driver - using standard SPI interface
 *
 */

/*--------------------------------------------------
 *                              Includes
 *--------------------------------------------------*/

#include "base.h"

/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/

/* driver states */
typedef enum
{
    CYPFLASH_ST_UNINITIALIZED  = 0u,    /* driver and device not initialized yet */
    CYPFLASH_ST_INITIALIZING,           /* driver and device under initialization phase (from uninit or from low power) */
    CYPFLASH_ST_IDLE,                   /* device is in idle state - no measurement result available */
    CYPFLASH_ST_READY,                  /* device is in idle state - measurement result is available. Mask is returned by _GetStatus function */
    CYPFLASH_ST_BUSY,                   /* device is busy with measurement */
    CYPFLASH_ST_ERROR                   /* device error - can not communicate with the device */
} tCypFlashStatus;

typedef struct
{ /* Status Register 1 Volatile */
  uint8 WIP:1;
  uint8 WEL:1;
  uint8 BP0:1;
  uint8 BP1:1;
  uint8 BP2:1;
  uint8 TBPROT:1;
  uint8 SEC:1;
  uint8 SRP0:1;
  /* Status Register 2 Volatile */
  uint8 PS:1;
  uint8 ES:1;
  uint8 RFU_1:3; /* reserved for future use bits */
  uint8 P_ERR:1;
  uint8 E_ERR:1;
  uint8 RFU_2:1; /* reserved for future use bits */
} tCypFlashDeviceStatus;

/*--------------------------------------------------
                                Defines
 *--------------------------------------------------*/



/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/

/* Initialize the driver */
void CypFlash_Init(void);

/* Main function call */
void CypFlash_MainFunction(void);

/* Get the status of the driver. */
tCypFlashStatus CypFlash_GetStatus(void);

/* Device handling functions */


/*  Data handling functions */



#endif
