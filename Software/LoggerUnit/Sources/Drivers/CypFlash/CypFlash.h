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
  CYPFLASH_ST_UNINITIALIZED  = 0u, /* driver and device not initialized yet */
  CYPFLASH_ST_INITIALIZING,        /* driver and device under initialization phase (from uninit or from low power) */
  CYPFLASH_ST_READY,               /* device is in ready state - all status and data from memory updated in driver */
  CYPFLASH_ST_BUSY,                /* device is busy with SPI communication or other command */
  CYPFLASH_ST_ERROR                /* device error - can not communicate with the device */
} tCypFlashStatus;

/*--------------------------------------------------
                                Defines
 *--------------------------------------------------*/



/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/

/* Generic driver interfaces */
void CypFlash_Init(void);
void CypFlash_MainFunction(void);
tCypFlashStatus CypFlash_GetStatus(void);

/* Device handling functions */
void CypFlash_Sleep(void);
void CypFlash_Wake(void);
void CypFlash_EraseAll(void);

/*  Data handling functions */
tCypFlashStatus CypFlash_Read(void);
tCypFlashStatus CypFlash_Write(void);
tCypFlashStatus CypFlash_WritePage(void);
tCypFlashStatus CypFlash_EraseSector(void); /* 4K sector */
tCypFlashStatus CypFlash_EraseBlock(void); /* 32K block */

#endif
