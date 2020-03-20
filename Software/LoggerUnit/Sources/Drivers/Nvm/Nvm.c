/* NVM and Storage Memory Manager */

/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include <string.h>
#include "base.h"
#include "HALPort.h"
#include "Nvm.h"
#include "Nvm_Cfg.h"

/*--------------------------------------------------
 *                  Defines and type definitions
 *--------------------------------------------------*/

#define NVM_BLOCKSTATE_DEFAULT      0
#define NVM_BLOCKSTATE_NVM          1
#define NVM_BLOCKSTATE_DIRTY        2
#define NVM_BLOCKSTATE_WR_MAIN      3   /* Written to the main sector, writing to mirror still ongoing */

typedef enum
{
    NVM_ST_INIT = 0,         /* Initialization */
    NVM_ST_INITHEADER,       /* Initialization of NVM Header */
    NVM_ST_INITDATA,         /* Initialization of NVM Data */
    NVM_ST_IDLE,             /* Nothing to do */
    NVM_ST_REPAIR,           /* Erase all sectors and rebuild nvm */
    NVM_ST_REPAIR_CNT,       /*      Continue */
    NVM_ST_ERROR             /* Flash memory unavailable */    
} tNvmDataInternalStates;

typedef struct
{
    uint8   MagicWord;          /* 0xFF - unused, 0xAA - current, 0x0A - history, 0x00 - invalid */
    uint8   BlockID;            /* Index of the block */
    uint16  InstanceNr;         /* Reset only with NVM sectors erase */
    uint16  BlockAbsAddress;    /* Absolute address of the block within current sector */
    uint16  CRC16;              /* CRC of the data area, also with elements from header:
                                 * (BlockID, InstaceNr, BlockAbsAddress) */
} tNvmBlockHeaderStruct;

typedef struct
{
    uint8                   RamMirror[NVM_TOTAL_SIZE_OF_BLOCKS];
    uint8                   Buffer[NVM_MAX_BLOCK_SIZE];
    uint8                   CurrentBlockInstance[NVM_NUMBER_OF_BLOCKS];
    tNvmBlockHeaderStruct   CurrentHeader;
    uint16                  CurrentHeadAddr;
    uint16                  CurrentDataAddr;
    uint8                   CurrentSector;
    tNvmDataInternalStates  InternalState;
    uint8                   BlockFound;
    uint8                   BlockState[NVM_NUMBER_OF_BLOCKS]; /* 0 - default, 1 - nv data, 2 - dirty */
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
                lNvm.InternalState = NVM_ST_INITHEADER;
            break;
        }
    case NVM_ST_INITHEADER:
        {
            if (Nvm_GetMemoryStatus() == RES_OK)
            {
                switch (lNvm.Buffer[0])
                {
                case 0x0A:  /* History data considered valid until newer data will be found */
                case 0xAA:  /* Valid header, save and process header information */
                    {                        
                        memcpy(&lNvm.CurrentHeader, lNvm.Buffer, NVM_HEADER_SIZE);
                        
                        /* Check plausibility of block ID and address  */
                        if (((lNvm.CurrentHeader.BlockAbsAddress & NVM_SECTOR_MASK) == (lNvm.CurrentHeadAddr & NVM_SECTOR_MASK))
                            && (lNvm.CurrentHeader.BlockID < NVM_NUMBER_OF_BLOCKS))
                        {       /* read out data associated with actual header */
                            Nvm_ReadMemory(lNvm.CurrentHeader.BlockAbsAddress, 
                                             cNvmBlockConfig[lNvm.CurrentHeader.BlockID].BlockSize, 
                                             lNvm.Buffer);
                            lNvm.InternalState = NVM_ST_INITDATA;
                        }
                        else
                        {       /* Invalid address within header */
                           local_NvmHandleCorruptMemory();
                        }
                        break;
                    }
                case 0x55:  /* NVM continues in next sector */
                    {
                        memcpy(&lNvm.CurrentHeader, lNvm.Buffer, NVM_HEADER_SIZE);
                        
                        if ( ((lNvm.CurrentSector==0)||(lNvm.CurrentSector==2))
                            &&(lNvm.CurrentHeader.BlockID == 0xA5)
                            &&(lNvm.CurrentHeader.InstanceNr == 0xCAFE)
                            &&(lNvm.CurrentHeader.BlockAbsAddress == 0xDADA)
                            &&(lNvm.CurrentHeader.CRC16 == 0x0CBC)
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
                case 0x00:  /* Invalidated header ignored, go to next header */
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
            if (Nvm_GetMemoryStatus() == RES_OK)
            {   /* Check header and block data consistency */
                if (  (lNvm.Buffer[0]==cNvmBlockConfig[lNvm.CurrentHeader.BlockID].MagicWord)
                    &&(CalcCRC16(lNvm.CurrentHeader.InstanceNr+lNvm.CurrentHeader.BlockAbsAddress+lNvm.CurrentHeader.BlockID,
                                 lNvm.Buffer, 
                                 cNvmBlockConfig[lNvm.CurrentHeader.BlockID].BlockSize) 
                       == lNvm.CurrentHeader.CRC16)
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
                    lNvm.CurrentDataAddr = lNvm.CurrentHeader.BlockAbsAddress;
                    
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
            
/* !!!!!!!!!!!!!!!!!!!!!!!!! to be removed after os interrupts will be fixed */
            if(Nvm_GetMemoryStatus() == RES_OK) lNvm.InternalState = NVM_ST_INIT;
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
        //local_NvmStartRepairProcess();
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

uint16 CalcCRC16(uint16 startval, uint8* buffer, uint16 counter)
{
    uint16 acc = startval, i;
    for (i=0; i<counter; i++) acc += (buffer[i]+i)*buffer[i];
    //return acc;
    return 0xFFFF;
}