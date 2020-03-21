/* NVM and Storage Memory Manager */

/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include <string.h>
#include "base.h"
#include "HALPort.h"
#include "LibCrc.h"
#include "Nvm.h"
#include "Nvm_Cfg.h"

/*--------------------------------------------------
 *                  Defines and type definitions
 *--------------------------------------------------*/

typedef enum
{
    NVM_ST_INIT = 0,        /* Initialization */
    NVM_ST_READHEADER,      /* Initialization of NVM Header */
    NVM_ST_INITDATA,        /* Initialization of NVM Data */
    NVM_ST_READDATA,        /* Read out block data associated with actual header */
    NVM_ST_WAIT,            /* Wait for last command to be finished */
    NVM_ST_IDLE,            /* Nothing to do */
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
    tNvmBlockHeaderStruct   CurrentHeader;
    uint8                   RamMirror[NVM_TOTAL_SIZE_OF_BLOCKS];
    uint8                   Buffer[NVM_MAX_BLOCK_SIZE];
    tNvmBlockState          BlockState[NVM_NUMBER_OF_BLOCKS];
    uint16                  CurrentBlockAddr[NVM_NUMBER_OF_BLOCKS];
    uint16                  CurrentHeadAddr;
    uint16                  CurrentDataAddr;
    uint8                   CurrentBlockInstance[NVM_NUMBER_OF_BLOCKS];
    uint8                   CurrentSector;
    uint8                   BlockFound;
} tNvmDataStruct;

/*--------------------------------------------------
 *                  Local Variables and prototypes
 *--------------------------------------------------*/

static tNvmDataStruct  lNvm;

/* NVM internal functions */
static void local_NvmHandleCorruptMemory(void);
static void local_NvmStartRepairProcess(void);

/* CRC */
uint16 CalcCRC16(uint16 startval, uint8* buffer, uint16 counter);
/*--------------------------------------------------
 *                  Interface Functions
 *--------------------------------------------------*/

void Nvm_Init(void)
{
    PORT_PIN_LED_ON_ON();
    /* initialize local variables */
    memset(&lNvm, 0, sizeof(tNvmDataStruct));
    /* Fill RAM mirror with default data */
    memcpy(&lNvm.RamMirror, &cNvmDefaultData, NVM_TOTAL_SIZE_OF_BLOCKS);
    
    if (Nvm_Init_Internal() == RES_OK)
    {
        while(lNvm.InternalState < NVM_ST_IDLE) Nvm_Main();
    }
    /* else Initialization has to be delayed after OS starts */
}

void Nvm_Main(void)
{
    switch(lNvm.InternalState)
    {   
    case NVM_ST_IDLE:
        {
            break;
        }
    case NVM_ST_INIT:
        {
            /* Parse header areas - start with Sector 0 */
            /* Start to check actual sector */
            if (Nvm_ReadMemory(lNvm.CurrentHeadAddr, NVM_HEADER_SIZE, lNvm.Buffer) == RES_OK)
                lNvm.InternalState = NVM_ST_READHEADER;
            break;
        }
    case NVM_ST_READHEADER:
        {
            if (Nvm_GetMemoryStatus() == RES_OK)
            {
                switch (lNvm.Buffer[0])
                {
                case NVM_HEADER_HISTORY:  /* History data considered valid until newer data will be found */
                case NVM_HEADER_VALID:  /* Valid header, save and process header information */
                    {                        
                        memcpy(&lNvm.CurrentHeader, lNvm.Buffer, NVM_HEADER_SIZE);
                        
                        /* Check plausibility of block ID and address  */
                        if ((((lNvm.CurrentHeader.BlockSegAddr*NVM_MIN_BLOCK_SIZE) & NVM_SECTOR_MASK) == (lNvm.CurrentHeadAddr & NVM_SECTOR_MASK))
                            && (lNvm.CurrentHeader.BlockID < NVM_NUMBER_OF_BLOCKS)
                            && ((lNvm.CurrentHeader.BlockSegAddr*NVM_MIN_BLOCK_SIZE)>lNvm.CurrentHeadAddr+NVM_HEADER_SIZE))
                        {       /* read out data associated with actual header */
                            lNvm.InternalState = NVM_ST_INITDATA;
                        }
                        else
                        {       /* Invalid address within header */
                           local_NvmHandleCorruptMemory();
                        }
                        break;
                    }
                case NVM_HEADER_NEXTSECTOR:  /* NVM continues in next sector */
                    {
                        memcpy(&lNvm.CurrentHeader, lNvm.Buffer, NVM_HEADER_SIZE);
                        
                        if ( ((lNvm.CurrentSector==0)||(lNvm.CurrentSector==2))
                            &&(lNvm.CurrentHeader.BlockID == NVM_NEXTSECT_BLOCKID)
                            &&(lNvm.CurrentHeader.InstanceNr == NVM_NEXTSECT_INSTANCE)
                            &&(lNvm.CurrentHeader.BlockSegAddr == NVM_NEXTSECT_BLOCKSEG)
                            &&(lNvm.CurrentHeader.CRC8 == NVM_NEXTSECT_CRC8)
                           )
                        {
                            lNvm.CurrentSector++;
                            lNvm.CurrentHeadAddr = lNvm.CurrentSector*NVM_SECTOR_SIZE;
                            lNvm.CurrentDataAddr = (lNvm.CurrentSector+1)*NVM_SECTOR_SIZE;
                            lNvm.InternalState = NVM_ST_INIT;
                        }
                        else
                        {   /* Invalid header data in the actual sector - header space corrupted, continue with mirror data */
                            local_NvmHandleCorruptMemory();
                        }
                        break;
                    }
                case NVM_HEADER_INVALID:  /* Invalidated header ignored, go to next header */
                    {
                        lNvm.CurrentHeadAddr += NVM_HEADER_SIZE;
                        lNvm.InternalState = NVM_ST_INIT;
                        break;
                    }
                case NVM_ERASE_VAL:  /* No more headers available in the actual sector, data OK */
                    {
                        if (lNvm.BlockFound>0) /* We already found data, finish */
                        {
                            PORT_PIN_LED_ON_OFF();
                            lNvm.InternalState = NVM_ST_IDLE;
                        }
                        else    /* Check other sectors if available */
                        {
                            if (lNvm.CurrentSector<NVM_SECTOR_CNT*NVM_DATASET_CNT-1)
                            {
                                lNvm.CurrentSector++;
                                lNvm.CurrentHeadAddr = lNvm.CurrentSector*NVM_SECTOR_SIZE;
                                lNvm.InternalState = NVM_ST_INIT;
                            }
                            else
                            {
                                /* No data available, stay with default data */
                                local_NvmStartRepairProcess();
                            }
                        }
                        break;
                    }
                default:    /* Invalid header data in the actual sector - header space corrupted, continue with mirror data */
                    {
                        local_NvmHandleCorruptMemory();
                    }
                }
            }
            /* else Flash memory still busy with the read operation, stay in the same state */
            break;
        }
    case NVM_ST_INITDATA:
        {
            if (Nvm_ReadMemory( (lNvm.CurrentHeader.BlockSegAddr*NVM_MIN_BLOCK_SIZE), 
                                cNvmBlockConfig[lNvm.CurrentHeader.BlockID].BlockSize, 
                                lNvm.Buffer) == RES_OK)
                lNvm.InternalState = NVM_ST_READDATA;
            break;
        }
    case NVM_ST_READDATA:
        {
            if (Nvm_GetMemoryStatus() == RES_OK)
            {   /* Check header and block data consistency */
                if (  (lNvm.Buffer[0]==cNvmBlockConfig[lNvm.CurrentHeader.BlockID].MagicWord)
                    &&(CalcCRC8(lNvm.CurrentHeader.InstanceNr+(lNvm.CurrentHeader.BlockSegAddr)+lNvm.CurrentHeader.BlockID,
                                 lNvm.Buffer, 
                                 cNvmBlockConfig[lNvm.CurrentHeader.BlockID].BlockSize) 
                       == lNvm.CurrentHeader.CRC8)
                    )
                {
                    /* Data valid, store in RAM Mirror */
                    memcpy(lNvm.RamMirror+cNvmBlockConfig[lNvm.CurrentHeader.BlockID].BlockAddress,
                           lNvm.Buffer,
                           cNvmBlockConfig[lNvm.CurrentHeader.BlockID].BlockSize);

                    /* Mark block state */
                    lNvm.BlockFound = 1;
                    lNvm.BlockState[lNvm.CurrentHeader.BlockID] = NVM_BLOCKSTATE_NVM;
                    
                    /* Update Instance number */
                    lNvm.CurrentBlockInstance[lNvm.CurrentHeader.BlockID] = lNvm.CurrentHeader.InstanceNr;
                    
                    /* Update global BlockAddress */
                    lNvm.CurrentDataAddr = (lNvm.CurrentHeader.BlockSegAddr*NVM_MIN_BLOCK_SIZE);
                    lNvm.CurrentBlockAddr[lNvm.CurrentHeader.BlockID] = lNvm.CurrentDataAddr;
                    
                    /* Parse next header */
                    lNvm.CurrentHeadAddr += NVM_HEADER_SIZE;
                    lNvm.InternalState = NVM_ST_INIT;
                }
                else        /* Data corrupted, go to mirror area */
                {
                    local_NvmHandleCorruptMemory();
                }
            }
            break;
        }
    case NVM_ST_REPAIR:
        {
            if (Nvm_StartEraseSector(lNvm.CurrentHeadAddr) == RES_OK)
                lNvm.InternalState = NVM_ST_REPAIR_CNT;
            break;
        }
    case NVM_ST_REPAIR_CNT:
        {
            if (Nvm_GetMemoryStatus() == RES_OK)
            {
                if (lNvm.CurrentSector<NVM_SECTOR_CNT*NVM_DATASET_CNT-1)
                {
                    lNvm.CurrentSector++;
                    lNvm.CurrentHeadAddr = lNvm.CurrentSector*NVM_SECTOR_SIZE;
                    lNvm.InternalState = NVM_ST_REPAIR;
                }
                else
                {
                    lNvm.CurrentSector = 0;
                    lNvm.CurrentHeadAddr = 0;
                    lNvm.InternalState = NVM_ST_IDLE;
                }
            }
            break;
        }
    case NVM_ST_ERROR:
        {   /* Error case - erase memory and rebuild data */
            local_NvmStartRepairProcess();
            break;
        }
    default:
        {
            /* Error case */
            lNvm.InternalState = NVM_ST_ERROR;
        }
    }
}

/*--------------------------------------------------
 *             Local functions
 *--------------------------------------------------*/

static void local_NvmHandleCorruptMemory(void)
{
    /* Invalid header data in the actual sector - header space corrupted, continue with mirror data */
    /* If current sector < 2, check the same mirror header area */
    if (lNvm.CurrentSector<NVM_DATASET_CNT)
    {
            lNvm.CurrentSector+=NVM_DATASET_CNT;
            lNvm.CurrentHeadAddr += NVM_DATASET_CNT*NVM_SECTOR_SIZE;
            lNvm.InternalState = NVM_ST_INIT;
    }
    else    /* Already in mirror flash space, start repair process */
    {
        local_NvmStartRepairProcess();
    }
}

static void local_NvmStartRepairProcess(void)
{
    uint32 i;
    
    lNvm.CurrentSector = 0u;
    lNvm.CurrentHeadAddr = 0u;
    lNvm.CurrentDataAddr = NVM_SECTOR_SIZE;
    
    /* Mark nvm based data as dirty */
    for (i=0; i<NVM_NUMBER_OF_BLOCKS; i++)
    {
        lNvm.CurrentBlockInstance[i] = 0u;
        if (lNvm.BlockState[i]==NVM_BLOCKSTATE_NVM)
            lNvm.BlockState[i]=NVM_BLOCKSTATE_DIRTY;
    }

    lNvm.InternalState = NVM_ST_REPAIR;
}

