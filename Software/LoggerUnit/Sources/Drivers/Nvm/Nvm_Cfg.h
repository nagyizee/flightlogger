#ifndef	_NVMCFG_H
#define _NVMCFG_H
/**
 *
 *
 *
 */

/*--------------------------------------------------
 *                              Includes
 *--------------------------------------------------*/
#include "base.h"
#include "CypFlash.h"
     
/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/

typedef struct
{
    uint16      BlockAddress;   /* Start address of the block in the NVM array */
    uint8       BlockSize;
    uint8       MagicWord;
}tNvmBlockStruct;

/*--------------------------------------------------
 *                              Defines
 *--------------------------------------------------*/

/* Underlying data storage method/device selection - only 1 should be active */
//#define NVM_USE_RAM         /* internal RAM emulated NVM - used for testing */
#define NVM_USE_CYPFLASH    /* Use Cypress S25FL064 external flash driver */

#define UC_LITTLE_ENDIAN    /* Comment out if moved to big endian microcontroller */
     
/* Block size settings */
#define NVM_NUMBER_OF_BLOCKS         3  /* Max 8 different blocks are allowed */
#define NVM_HEADER_SIZE              4  /* Bytes */
#define NVM_MIN_BLOCK_SIZE           8  /* Bytes */
/* !!! All block sizes have to be multiples of the min block size !!! */
#define NVM_MAX_BLOCK_SIZE          16  /* Bytes */
#define NVM_TOTAL_SIZE_OF_BLOCKS    32  /* Only data blocks, without header info! */

#define NVM_DATASET_CNT              2  /* 1 - no mirror / 2 - use mirror */
#define NVM_SECTOR_CNT               2  /* Number of sectors used for NVM */

/* Magic numbers */
#define NVM_ERASE_VAL             0xFF  /* Default data after memory erased */
#define NVM_HEADER_VALID          0xAA  /* Valid block data signal in header */
#define NVM_HEADER_HISTORY        0x0A  /* History or obsolete data */
#define NVM_HEADER_INVALID        0x00  /* Data invalidated */

#define NVM_HEADER_NEXTSECTOR     0x55  /*  8 bits - Data chain continues in next sector */
#define NVM_NEXTSECT_BLOCKID      0x07  /*  3 bits only */
#define NVM_NEXTSECT_INSTANCE     0x02  /*  3 bits only */
#define NVM_NEXTSECT_BLOCKSEG     0x2A  /* 10 bits only */
#define NVM_NEXTSECT_CRC8         0xCC  /*  8 bits */

/* Memory descriptors */
#ifdef NVM_USE_RAM
    #define NVM_SECTOR_SIZE              0x200
    #define NVM_SECTOR_MASK             0xFE00
#else
#ifdef NVM_USE_CYPFLASH
    #define NVM_SECTOR_SIZE             0x1000
    #define NVM_SECTOR_MASK             0xF000
#endif
#endif

/* Verify if segment size in header address fits to sector size + min block size */     
#if NVM_SECTOR_SIZE/(NVM_HEADER_SIZE+NVM_MIN_BLOCK_SIZE)>1023
#error     
#endif
     
/*--------------------------------------------------
 *                              Exported config data
 *--------------------------------------------------*/

extern const tNvmBlockStruct cNvmBlockConfig[NVM_NUMBER_OF_BLOCKS];
extern const uint8 cNvmDefaultData[NVM_TOTAL_SIZE_OF_BLOCKS];

/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/
tResult Nvm_Init_Internal(void);

tResult Nvm_GetMemoryStatus(void);
tResult Nvm_StartEraseSector(uint32 address);
tResult Nvm_ReadMemory(uint32 address, uint16 count, uint8* buffer);
tResult Nvm_WriteMemory(uint32 address, uint16 count, uint8* buffer);

#endif

