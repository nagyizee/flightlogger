#ifndef	_NVMINTERNALS_H
#define _NVMINTERNALS_H
/**
 *  Internal definitions of the NVM module
 */

/*--------------------------------------------------
 *                              Includes
 *--------------------------------------------------*/

#include "Nvm.h"
#include "Nvm_Cfg.h"

/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/

typedef enum
{
    NVM_ST_INIT = 0,        /* Initialization */
    NVM_ST_READHEADER,      /* Initialization of NVM Header */
    NVM_ST_INITDATA,        /* Initialization of NVM Data */
    NVM_ST_READDATA,        /* Read out block data associated with actual header */
    NVM_ST_IDLE,            /* Nothing to do */
#ifdef NVM_READ_BEFORE_WRITE
    NVM_ST_CHECKERASED,     /* Check if memory is properly erased before writing */
#endif
    NVM_ST_WRITEHEADER,     /* Writing the header info to current sector */
    NVM_ST_WRITEDATA,       /* Writing data to current sector */
    NVM_ST_WRITEHIST,       /* Update header magic word of previous data to mark it as history */
    NVM_ST_CHECKSECTORFULL, /* Check if current sector is full and next sector free */
    NVM_ST_WRITESECTORFULL, /* Write special header to follow NVM in next sector */
    NVM_ST_REPAIR,          /* Erase all sectors and rebuild nvm */
    NVM_ST_REPAIR_CNT,      /*      Continue */
    NVM_ST_ERROR            /* Flash memory unavailable */    
} tNvmDataInternalStates;

#ifdef UC_LITTLE_ENDIAN
typedef struct
{
    uint32  MagicWord    :  8;  /* 0xFF - unused, 0xAA - current, 0x0A - history, 0x00 - invalid */
    uint32  BlockID      :  3;  /* Max 8 different blocks can be handled */
    uint32  InstanceNr   :  3;  /* instance of the block in memory history, rollover when overflowing, starting with 1 */
    uint32  BlockSegAddr : 10;  /* Real address = Segment Address * NVM_MIN_BLOCK_SIZE */
    uint32  CRC8 :          8;           /* CRC of the data area, also with elements from header:
                                 * (BlockID, InstaceNr, BlockSegAddr) */
} tNvmBlockHeaderStruct;
#else
typedef struct
{
    uint32  CRC8         :  8;  /* CRC of the data area, also with elements from header:
                                 * (BlockID, InstaceNr, BlockSegAddr) */
    uint32  BlockSegAddr : 10;  /* Real address = Segment Address * NVM_MIN_BLOCK_SIZE */
    uint32  InstanceNr   :  3;  /* instance of the block in memory history, rollover when overflowing, starting with 1 */
    uint32  BlockID      :  3;  /* Max 8 different blocks can be handled */
    uint32  MagicWord    :  8;  /* 0xFF - unused, 0xAA - current, 0x0A - history, 0x00 - invalid */
} tNvmBlockHeaderStruct;
#endif

typedef struct
{
    tNvmDataInternalStates  InternalState;
#ifdef NVM_READ_BEFORE_WRITE
    tNvmDataInternalStates  IntNextState;
#endif
    tNvmBlockHeaderStruct   CurrentHeader;
    uint8                   RamMirror[NVM_TOTAL_SIZE_OF_BLOCKS];
    uint8                   Buffer[NVM_MAX_BLOCK_SIZE];
    tNvmBlockStatus         BlockStatus[NVM_NUMBER_OF_BLOCKS];
    uint16                  LastHeadAddr[NVM_NUMBER_OF_BLOCKS];
    uint16                  CalcDataAddr;
    uint16                  CurrentHeadAddr;
    uint16                  CurrentDataAddr;
#ifdef NVM_READ_BEFORE_WRITE
    uint16                  CheckCount;
#endif
    uint8                   CurrentBlockInstance[NVM_NUMBER_OF_BLOCKS];
    uint8                   CurrentSector;
    uint8                   MemoryStatus;
} tNvmDataStruct;

/*--------------------------------------------------
 *                              Defines
 *--------------------------------------------------*/

/*--------------------------------------------------
 *                           NVM internal interfaces
 *--------------------------------------------------*/

/* NVM internal functions */
static void local_NvmHandleInitCorruptMemory(void);
static void local_NvmHandleWriteCorruptMemory(void);

static void local_NvmStartRepairProcess(void);

#ifdef NVM_READ_BEFORE_WRITE
static void local_NvmReadBeforeWrite(uint16 address, uint16 count);
static tResult local_NvmCheckErased(void);
#endif

#endif

