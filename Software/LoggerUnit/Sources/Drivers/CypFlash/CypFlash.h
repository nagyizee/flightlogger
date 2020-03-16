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
  CYPFLASH_ST_PARAM_ERROR,         /* function parameter error */
  CYPFLASH_ST_COMM_ERROR,          /* can not communicate with the device, retry */
  CYPFLASH_ST_DEVICE_ERROR         /* device fatal error - driver blocked */
} tCypFlashStatus;

/*--------------------------------------------------
                                Defines
 *--------------------------------------------------*/

#define CYPFLASH_READ_BUFSIZE   256 /* Bytes */
#define CYPFLASH_WRITE_BUFSIZE  256 /* MUST BE 256 Bytes !! */

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

/*  Data handling functions */
tCypFlashStatus CypFlash_Read(uint32 address, uint16 count, uint8* buffer);
tCypFlashStatus CypFlash_Write(uint32 address, uint16 count, uint8* buffer);
tCypFlashStatus CypFlash_WritePage(uint32 address, uint8* buffer);
tCypFlashStatus CypFlash_EraseSector(uint32 address); /* 4K sector */
tCypFlashStatus CypFlash_EraseBlock(uint32 address); /* 64K block */
tCypFlashStatus CypFlash_EraseAll(void);

#endif
