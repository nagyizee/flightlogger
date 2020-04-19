/* NVM and Storage Memory Manager */

/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include <string.h>
#include "HALPort.h"
#include "LibCrc.h"
#include "Nvm_Internals.h"

/*--------------------------------------------------
 *                  Defines and type definitions
 *--------------------------------------------------*/

/*--------------------------------------------------
 *                  Local Variables and prototypes
 *--------------------------------------------------*/

static tNvmDataStruct  lNvm;

static uint8 local_calc_CRC8(uint8* buffer);

/*--------------------------------------------------
 *                  Interface Functions
 *--------------------------------------------------*/

void Nvm_Init(void)
{
    PORT_PIN_LED_ON_ON();
    /* initialize local variables */
    memset(&lNvm, 0, sizeof(tNvmDataStruct));
    
    lNvm.CurrentDataAddr = (lNvm.CurrentSector+1)*NVM_SECTOR_SIZE;

    /* Fill RAM mirror with default data */
    memcpy(&lNvm.RamMirror, &cNvmDefaultData, NVM_TOTAL_SIZE_OF_BLOCKS);
    
    /* Fill LastHeadaddr[] with 0xFF */
    memset(&lNvm.LastHeadAddr[0], 0xFF, sizeof(lNvm.LastHeadAddr));
    
    if (Nvm_InitInternal() == RES_OK)
    {
        while(lNvm.InternalState < NVM_ST_IDLE) 
        {
            Nvm_Main();
            Nvm_MemoryMain(2); /* 100 calls timeout */
        }
    }
    /* else Initialization has to be delayed after OS starts */
}

void Nvm_Main(void)
{
    switch(lNvm.InternalState)
    {   
    case NVM_ST_IDLE:
        {
            uint32 i = 0;
            if (Nvm_GetMemoryStatus() == RES_OK) /* Memory not busy */
            {
                while ((i<NVM_NUMBER_OF_BLOCKS) && (lNvm.BlockStatus[i] < NVM_BLOCKSTATE_DIRTY)) i++;
                if (i < NVM_NUMBER_OF_BLOCKS)
                {
                        /* Set up global variables for writing */
                    lNvm.CurrentDataAddr -= cNvmBlockConfig[i].BlockSize;
                        /* Prepare header info to write current block */
                    lNvm.CurrentHeader.MagicWord = NVM_HEADER_VALID;
                    lNvm.CurrentHeader.BlockID = i;
                    lNvm.CurrentHeader.InstanceNr = (lNvm.CurrentBlockInstance[i] + 1) & 0x7; /* 3 bits available, overflow to 0 */
                    lNvm.CurrentHeader.BlockSegAddr = (lNvm.CurrentDataAddr & NVM_INNER_MASK) / NVM_MIN_BLOCK_SIZE;
                    lNvm.CurrentHeader.CRC8 = local_calc_CRC8(&lNvm.RamMirror[cNvmBlockConfig[i].BlockAddress]);
                        /* Safeguard data to write buffer to keep consistency with header CRC */
                    memcpy(lNvm.WriteMirror, lNvm.RamMirror+cNvmBlockConfig[i].BlockAddress, cNvmBlockConfig[i].BlockSize);
                    lNvm.WriteStatus[i] = lNvm.BlockStatus[i];
                    lNvm.BlockStatus[i] = NVM_BLOCKSTATE_NVM;
                    
#ifdef NVM_READ_BEFORE_WRITE
                    lNvm.InternalState = NVM_ST_CHECKERASED;
                    lNvm.IntNextState = NVM_ST_WRITEHEADER;
                    /* Check if memory available */
                    local_NvmReadBeforeWrite(lNvm.CurrentHeadAddr, NVM_HEADER_SIZE);
#else
                    lNvm.InternalState = NVM_ST_WRITEHEADER;
                    Nvm_WriteMemory(lNvm.CurrentHeadAddr, NVM_HEADER_SIZE, (uint8*) &lNvm.CurrentHeader);
#endif
                }
                //else nothing to do, stay in the same state NVM_ST_IDLE
            }
            //else memory still busy with operation, stay in the same state NVM_ST_IDLE
            break;
        }
    case NVM_ST_INIT:
        {   /* Parse header areas - start with Sector 0 */
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
                case NVM_HEADER_VALID:      /* Valid header, process header information */
                case NVM_HEADER_HISTORY:    /* History data considered valid until newer data will be found */
                case NVM_HEADER_INVALID:    /* Invalidated data, check header data consistency and update addresses */
                    {                        
                        memcpy(&lNvm.CurrentHeader, lNvm.Buffer, NVM_HEADER_SIZE);
                            /* Check plausibility of block ID */
                        if (lNvm.CurrentHeader.BlockID < NVM_NUMBER_OF_BLOCKS)
                        {   /* Check plausibility of BlockSegAddr */
                            /* Calculate data address from header offset + segment + current sector info */
                            lNvm.CalcDataAddr = (lNvm.CurrentHeader.BlockSegAddr*NVM_MIN_BLOCK_SIZE)
                                               +(lNvm.CurrentSector*NVM_SECTOR_SIZE);
                            if (  ((lNvm.CalcDataAddr & NVM_SECTOR_MASK) == (lNvm.CurrentHeadAddr & NVM_SECTOR_MASK))
                                &&(lNvm.CalcDataAddr > lNvm.CurrentHeadAddr+NVM_HEADER_SIZE))
                            {
                                if (lNvm.CalcDataAddr == lNvm.CurrentDataAddr-cNvmBlockConfig[lNvm.CurrentHeader.BlockID].BlockSize)
                                {   /* Update current block address pointer */
                                    lNvm.CurrentDataAddr =lNvm.CalcDataAddr;

                                    if (lNvm.CurrentHeader.MagicWord != NVM_HEADER_INVALID)
                                    {   /* read out data associated with actual header */
                                        lNvm.InternalState = NVM_ST_INITDATA;
                                    }
                                    else
                                    {   /* Skip data and go to next header */
                                        lNvm.CurrentHeadAddr += NVM_HEADER_SIZE;
                                        lNvm.InternalState = NVM_ST_INIT;
                                    }
                                }
                                else
                                {   /* Inconsistent header and block memory space */
                                    local_NvmHandleInitCorruptMemory(NVM_MEM_ST_HEADER_ERROR);
                                }
                            }
                            else
                            {       /* Invalid address within header */
                               local_NvmHandleInitCorruptMemory(NVM_MEM_ST_HEADER_ERROR);
                            }
                        }
                        else
                        {   /* Invalid block ID within header */
                            local_NvmHandleInitCorruptMemory(NVM_MEM_ST_HEADER_ERROR);
                        }
                        break;
                    }
                case NVM_HEADER_NEXTSECTOR:  /* NVM continues in next sector */
                    {
                        memcpy(&lNvm.CurrentHeader, lNvm.Buffer, NVM_HEADER_SIZE);
                        
                        if (((lNvm.CurrentSector < NVM_SECTOR_CNT-1 ) /* Not valid in the last sector */
#if (NVM_DATASET_CNT == 2)   /* Check also first sectors of mirror space */
                              ||(  (lNvm.CurrentSector >= NVM_SECTOR_CNT)
                                 &&(lNvm.CurrentSector < NVM_SECTOR_CNT*NVM_DATASET_CNT-1))
#endif
                            )
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
                            local_NvmHandleInitCorruptMemory(NVM_MEM_ST_HEADER_ERROR);
                        }
                        break;
                    }
                case NVM_ERASE_VAL:  /* No more headers available in the actual sector, data OK */
                    {
                        if ((lNvm.MemoryStatus & NVM_MEM_ST_BLOCKFOUND) != 0) /* We already found data, finish */
                        {
                                PORT_PIN_LED_ON_OFF();  /* LED switches off */
                                    /* Check if memory full */
                            if ((lNvm.CurrentDataAddr - lNvm.CurrentHeadAddr) < NVM_MAX_BLOCK_SIZE+2*NVM_HEADER_SIZE)
                            {
                                if (lNvm.CurrentSector == NVM_SECTOR_CNT-1)
                                {       /* Memory full */
                                    lNvm.MemoryStatus |= NVM_MEM_ST_STORAGE_FULL;
                                    lNvm.InternalState = NVM_ST_ERROR;
                                }
                                else
                                {       /* Missing Next sector header, abort Init */
                                    local_NvmHandleInitCorruptMemory(NVM_MEM_ST_HEADER_ERROR);
                                }
                            }
                            else
                            {
                                lNvm.InternalState = NVM_ST_IDLE;
                            }
                        }
                        else    /* Check other sectors if available */
                        {
                            if (lNvm.CurrentSector<NVM_SECTOR_CNT*NVM_DATASET_CNT-1)
                            {
                                lNvm.CurrentSector++;
                                lNvm.CurrentHeadAddr = lNvm.CurrentSector*NVM_SECTOR_SIZE;
                                lNvm.CurrentDataAddr = (lNvm.CurrentSector+1)*NVM_SECTOR_SIZE;
                                lNvm.InternalState = NVM_ST_INIT;
                            }
                            else
                            {
                                PORT_PIN_LED_ON_OFF();
                                /* No data available, stay with default data */
                                lNvm.CurrentSector = 0;
                                lNvm.CurrentHeadAddr = 0;
                                lNvm.CurrentDataAddr = NVM_SECTOR_SIZE;
                                lNvm.InternalState = NVM_ST_IDLE;
                            }
                        }
                        break;
                    }
                default:    /* Invalid header data in the actual sector - header space corrupted, continue with mirror data */
                    {
                        local_NvmHandleInitCorruptMemory(NVM_MEM_ST_HEADER_ERROR);
                    }
                }
            }
            /* else Flash memory still busy with the read operation, stay in the same state */
            break;
        }
    case NVM_ST_INITDATA:
        {
            if (Nvm_ReadMemory( (lNvm.CurrentSector*NVM_SECTOR_SIZE)
                               +(lNvm.CurrentHeader.BlockSegAddr*NVM_MIN_BLOCK_SIZE), 
                                cNvmBlockConfig[lNvm.CurrentHeader.BlockID].BlockSize, 
                                lNvm.Buffer) == RES_OK)
                lNvm.InternalState = NVM_ST_READDATA;
            break;
        }
    case NVM_ST_READDATA:
        {
            if (Nvm_GetMemoryStatus() == RES_OK)
            {   /* Check header and block data consistency */
                if (  (lNvm.Buffer[0] == cNvmBlockConfig[lNvm.CurrentHeader.BlockID].MagicWord)
                    &&(local_calc_CRC8(lNvm.Buffer) == lNvm.CurrentHeader.CRC8)
                   )
                {
                    /* Data valid, store in RAM Mirror */
                    memcpy(lNvm.RamMirror+cNvmBlockConfig[lNvm.CurrentHeader.BlockID].BlockAddress,
                           lNvm.Buffer,
                           cNvmBlockConfig[lNvm.CurrentHeader.BlockID].BlockSize);

                    /* Mark block state */
                    lNvm.MemoryStatus |= NVM_MEM_ST_BLOCKFOUND;
                    lNvm.BlockStatus[lNvm.CurrentHeader.BlockID] = NVM_BLOCKSTATE_NVM;
                    lNvm.WriteStatus[lNvm.CurrentHeader.BlockID] = NVM_BLOCKSTATE_NVM;
                    
                    /* Update Instance number */
                    lNvm.CurrentBlockInstance[lNvm.CurrentHeader.BlockID] = lNvm.CurrentHeader.InstanceNr;
                    
                    /* Go back from mirror to main memory area if recovering from corrupt main data */
                    if (lNvm.CurrentSector > NVM_SECTOR_CNT)
                    {
                        lNvm.CurrentSector -= NVM_SECTOR_CNT;
                        lNvm.CurrentHeadAddr -= NVM_SECTOR_CNT*NVM_SECTOR_SIZE;
                    }
                    lNvm.LastHeadAddr[lNvm.CurrentHeader.BlockID] = lNvm.CurrentHeadAddr;
                    lNvm.CurrentHeadAddr += NVM_HEADER_SIZE;
                    lNvm.InternalState = NVM_ST_INIT;
                }
                else        /* Data corrupted, go to mirror area */
                {
                    local_NvmHandleInitCorruptMemory(NVM_MEM_ST_DATA_ERROR);
                }
            }
            break;
        }
#ifdef NVM_READ_BEFORE_WRITE
    case NVM_ST_CHECKERASED:
        {
            if (Nvm_GetMemoryStatus() == RES_OK)
                if (local_NvmCheckErased() == RES_OK)
                {       /* Set next internal state */
                    lNvm.InternalState = lNvm.IntNextState;
                    
                    switch (lNvm.IntNextState)
                    {
                    case NVM_ST_WRITEHEADER:
                    case NVM_ST_WRITESECTORFULL:
                        {       /* Start header write process */
                            Nvm_WriteMemory(lNvm.CurrentHeadAddr, NVM_HEADER_SIZE, (uint8*) &lNvm.CurrentHeader);
                            break;
                        }
                    case NVM_ST_WRITEDATA:
                        {       /* Start data write process */
                            Nvm_WriteMemory(lNvm.CurrentDataAddr, 
                                    cNvmBlockConfig[lNvm.CurrentHeader.BlockID].BlockSize, 
                                    lNvm.WriteMirror);

                                /* Prepare the history mark data to be written in the generic buffer for the old header of the same block */
                            lNvm.Buffer[0] = NVM_HEADER_HISTORY;
                            break;
                        }
                    default:
                        {       /* Error case in state machine, memory overwrite? */
                            lNvm.InternalState = NVM_ST_ERROR;
                        }
                    }
                }
                else
                {       /* Memory not erased properly or NVM dedicated address space violated */
                    local_NvmHandleWriteCorruptMemory();
                }
            break;
        }
#endif
    case NVM_ST_WRITEHEADER:
        {       /* If possible check block data for virgin state, else wait for memory to be ready */
            if (Nvm_GetMemoryStatus() == RES_OK)
            {
#ifdef NVM_READ_BEFORE_WRITE                    
                lNvm.InternalState = NVM_ST_CHECKERASED;
                lNvm.IntNextState = NVM_ST_WRITEDATA;
                
                local_NvmReadBeforeWrite(lNvm.CurrentDataAddr, cNvmBlockConfig[lNvm.CurrentHeader.BlockID].BlockSize);
#else
                lNvm.InternalState = NVM_ST_WRITEDATA;

                Nvm_WriteMemory(lNvm.CurrentDataAddr, 
                        cNvmBlockConfig[lNvm.CurrentHeader.BlockID].BlockSize, 
                        lNvm.WriteMirror);

                    /* Prepare the history mark data to be written in the generic buffer for the old header of the same block */
                lNvm.Buffer[0] = NVM_HEADER_HISTORY;
#endif
            }
            break;
        }
    case NVM_ST_WRITEDATA:
        {       /* If possible set history flag in old header, else wait for memory to be ready */
            if (Nvm_GetMemoryStatus() == RES_OK)
            {
                if (lNvm.LastHeadAddr[lNvm.CurrentHeader.BlockID] != 0xFFFF)
                {
                    Nvm_WriteMemory(lNvm.LastHeadAddr[lNvm.CurrentHeader.BlockID], 
                                    1, 
                                    lNvm.Buffer);
                }
                //else skip setting history info in old header
                lNvm.InternalState = NVM_ST_WRITEHIST;
            }
            break;
        }
    case NVM_ST_WRITEHIST:
        {
            if (Nvm_GetMemoryStatus() == RES_OK)
            {
                lNvm.MemoryStatus |= NVM_MEM_ST_BLOCKFOUND;
                
#if NVM_DATASET_CNT == 2    /* Mirror data exist, write second dataset */
                if (lNvm.CurrentSector < NVM_SECTOR_CNT)
                {       /* Start write in mirror sector also */
                    lNvm.WriteStatus[lNvm.CurrentHeader.BlockID] = NVM_BLOCKSTATE_WR_MAIN;

                    lNvm.CurrentSector   += NVM_SECTOR_CNT;
                    lNvm.CurrentHeadAddr += NVM_SECTOR_CNT*NVM_SECTOR_SIZE;
                    lNvm.CurrentDataAddr += NVM_SECTOR_CNT*NVM_SECTOR_SIZE;
                    if (lNvm.LastHeadAddr[lNvm.CurrentHeader.BlockID] != 0xFFFF)
                    {
                        lNvm.LastHeadAddr[lNvm.CurrentHeader.BlockID] += NVM_SECTOR_CNT*NVM_SECTOR_SIZE;
                    }
    #ifdef NVM_READ_BEFORE_WRITE
                    lNvm.InternalState = NVM_ST_CHECKERASED;
                    lNvm.IntNextState = NVM_ST_WRITEHEADER;
                    /* Check if memory available */
                    local_NvmReadBeforeWrite(lNvm.CurrentHeadAddr, NVM_HEADER_SIZE);
    #else
                    lNvm.InternalState = NVM_ST_WRITEHEADER;
                    Nvm_WriteMemory(lNvm.CurrentHeadAddr, NVM_HEADER_SIZE, (uint8*) &lNvm.CurrentHeader);
    #endif
                }
                else    /* Mirror area written, finish process */
                {
                    lNvm.CurrentSector   -= NVM_SECTOR_CNT;
                    lNvm.CurrentHeadAddr -= NVM_SECTOR_CNT*NVM_SECTOR_SIZE;
                    lNvm.CurrentDataAddr -= NVM_SECTOR_CNT*NVM_SECTOR_SIZE;
 
                    lNvm.LastHeadAddr[lNvm.CurrentHeader.BlockID] = lNvm.CurrentHeadAddr;
                    lNvm.CurrentBlockInstance[lNvm.CurrentHeader.BlockID] = lNvm.CurrentHeader.InstanceNr;
                    lNvm.WriteStatus[lNvm.CurrentHeader.BlockID] = NVM_BLOCKSTATE_NVM;

                    lNvm.InternalState = NVM_ST_CHECKSECTORFULL;
                }
#else                   /* No mirror space, skip to sector full check */
                lNvm.LastHeadAddr[lNvm.CurrentHeader.BlockID] = lNvm.CurrentHeadAddr;
                lNvm.CurrentBlockInstance[lNvm.CurrentHeader.BlockID] = lNvm.CurrentHeader.InstanceNr;
                lNvm.WriteStatus[lNvm.CurrentHeader.BlockID] = NVM_BLOCKSTATE_NVM;
                
                lNvm.InternalState = NVM_ST_CHECKSECTORFULL;
#endif
            }
            break;
        }
    case NVM_ST_CHECKSECTORFULL:
        {
            lNvm.CurrentHeadAddr += NVM_HEADER_SIZE;
                    /* Check if current sector full */
            if (lNvm.CurrentDataAddr - lNvm.CurrentHeadAddr < NVM_MAX_BLOCK_SIZE+2*NVM_HEADER_SIZE)
            {       /* Check if there are empty sectors available */
                if (lNvm.CurrentSector < NVM_SECTOR_CNT-1)
                {       /* Signal following with next sector */
                    
                        /* Prepare header info to write current block */
                    lNvm.CurrentHeader.MagicWord = NVM_HEADER_NEXTSECTOR;
                    lNvm.CurrentHeader.BlockID = NVM_NEXTSECT_BLOCKID;
                    lNvm.CurrentHeader.InstanceNr = NVM_NEXTSECT_INSTANCE;
                    lNvm.CurrentHeader.BlockSegAddr = NVM_NEXTSECT_BLOCKSEG;
                    lNvm.CurrentHeader.CRC8 = NVM_NEXTSECT_CRC8;
                    
#ifdef NVM_READ_BEFORE_WRITE
                    lNvm.InternalState = NVM_ST_CHECKERASED;
                    lNvm.IntNextState = NVM_ST_WRITESECTORFULL;
                    /* Check if memory available */
                    local_NvmReadBeforeWrite(lNvm.CurrentHeadAddr, NVM_HEADER_SIZE);
#else
                    lNvm.InternalState = NVM_ST_WRITESECTORFULL;
                    Nvm_WriteMemory(lNvm.CurrentHeadAddr, NVM_HEADER_SIZE, (uint8*) &lNvm.CurrentHeader);
#endif
                }
                else
                {
                    lNvm.MemoryStatus |= NVM_MEM_ST_STORAGE_FULL;
                    lNvm.InternalState = NVM_ST_ERROR;
                }
            }
            else
            {
                lNvm.InternalState = NVM_ST_IDLE;
            }
            break;
        }
    case NVM_ST_WRITESECTORFULL:
        {
            if (Nvm_GetMemoryStatus() == RES_OK)
            {
#if NVM_DATASET_CNT == 2
                if (lNvm.CurrentSector<NVM_SECTOR_CNT)
                {
                        /* Write the special header in mirror memory */
                    lNvm.CurrentSector   += NVM_SECTOR_CNT;
                    lNvm.CurrentHeadAddr += NVM_SECTOR_CNT*NVM_SECTOR_SIZE;

    #ifdef NVM_READ_BEFORE_WRITE
                    lNvm.InternalState = NVM_ST_CHECKERASED;
                    lNvm.IntNextState = NVM_ST_WRITESECTORFULL;
                    /* Check if memory available */
                    local_NvmReadBeforeWrite(lNvm.CurrentHeadAddr, NVM_HEADER_SIZE);
    #else
                    lNvm.InternalState = NVM_ST_WRITESECTORFULL;
                    Nvm_WriteMemory(lNvm.CurrentHeadAddr, NVM_HEADER_SIZE, 
                                    (uint8*) &lNvm.CurrentHeader);
    #endif
                }
                else    /* Already in mirror flash space */
                {       /* Both main and mirror memory space are written */
                    lNvm.CurrentSector   -= NVM_SECTOR_CNT;
                    lNvm.CurrentHeadAddr -= NVM_SECTOR_CNT*NVM_SECTOR_SIZE;

                    lNvm.CurrentSector++;
                    lNvm.CurrentHeadAddr = lNvm.CurrentSector*NVM_SECTOR_SIZE;
                    lNvm.CurrentDataAddr = (lNvm.CurrentSector+1)*NVM_SECTOR_SIZE;

                    lNvm.InternalState = NVM_ST_IDLE;
                }
#else
                lNvm.CurrentSector++;
                lNvm.CurrentHeadAddr = lNvm.CurrentSector*NVM_SECTOR_SIZE;
                lNvm.CurrentDataAddr = (lNvm.CurrentSector+1)*NVM_SECTOR_SIZE;
                
                lNvm.InternalState = NVM_ST_IDLE;
#endif
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
        {   /* Error case - block driver until reset and reinitialization or purge history command */
                /* Keep LED static ON */
            PORT_PIN_LED_ON_ON();
            break;
        }
    default:
        {
            /* Error case in state machine, memory overwrite? */
            lNvm.InternalState = NVM_ST_ERROR;
        }
    }
}

tNvmStatus Nvm_GetStatus(void)
{
    return lNvm.MemoryStatus;
}

void Nvm_PurgeHistory(void)
{
    local_NvmStartRepairProcess();
}

tNvmBlockStatus Nvm_ReadBlock(uint8 blockID, uint8* buffer, uint16 count)
{   /* Check parameters */
    tNvmBlockStatus retval = NVM_PARAM_ERROR;
        
    if (  (blockID < NVM_NUMBER_OF_BLOCKS)
        &&(buffer != NULL)
        &&(count == cNvmBlockConfig[blockID].BlockSize))
    {
        memcpy(buffer, &lNvm.RamMirror[cNvmBlockConfig[blockID].BlockAddress], count);
        retval = lNvm.WriteStatus[blockID];
    }
    return retval;
}
tNvmBlockStatus Nvm_WriteBlock(uint8 blockID, uint8* buffer, uint16 count)
{
   /* Check parameters */
    tNvmBlockStatus retval = NVM_PARAM_ERROR;
    uint16 i = 0;
        
    if (  (blockID < NVM_NUMBER_OF_BLOCKS)
        &&(buffer != NULL)
        &&(count == cNvmBlockConfig[blockID].BlockSize)
        &&(buffer[0] == cNvmBlockConfig[blockID].MagicWord) )
    {
        /* Check if there is any change in data */
        while ((i<count) && (buffer[i] == lNvm.RamMirror[cNvmBlockConfig[blockID].BlockAddress+i])) i++;
        if (i<count)
        {       /* Change in block data compared to existing values */
            memcpy(&lNvm.RamMirror[cNvmBlockConfig[blockID].BlockAddress], buffer, count);
            
            if ((lNvm.MemoryStatus & NVM_MEM_ST_STORAGE_FULL) == 0)
            {
                    /* Keep previous state in return value to signal udating of non-written yet data */
                retval = lNvm.BlockStatus[blockID];
            }
            else
            {
                retval = NVM_MEMORY_FULL;
            }
                /* Update status after setting the return value */
            lNvm.BlockStatus[blockID] = NVM_BLOCKSTATE_DIRTY;
        }
        else
        {
            retval = NVM_DATA_UNCHANGED;
        }
    }
    /* else retval = NVM_PARAM_ERROR default value */
    return retval;
}

/*--------------------------------------------------
 *             Local functions
 *--------------------------------------------------*/

static void local_NvmHandleInitCorruptMemory(uint8 errsource)
{
    /* Invalid header or block data in the actual sector - continue with mirror data */
    lNvm.MemoryStatus |= errsource;
    /* If current sector < 2, check the same mirror header area */
#if NVM_DATASET_CNT == 2
    if (lNvm.CurrentSector<NVM_SECTOR_CNT)
    {
            /* Try the same header address in mirror memory */
        lNvm.CurrentSector   += NVM_SECTOR_CNT;
        lNvm.CurrentHeadAddr += NVM_SECTOR_CNT*NVM_SECTOR_SIZE;
        lNvm.InternalState = NVM_ST_INIT;
    }
    else    /* Already in mirror flash space */
    {       /* Both main and mirror memory space are corrupted */
        lNvm.InternalState = NVM_ST_ERROR;
    }
#else
    lNvm.InternalState = NVM_ST_ERROR;
#endif
}

static void local_NvmHandleWriteCorruptMemory(void)
{
    lNvm.InternalState = NVM_ST_ERROR;
}

static void local_NvmStartRepairProcess(void)
{
    uint32 i;
    
    lNvm.CurrentSector = 0u;
    lNvm.CurrentHeadAddr = 0u;
    lNvm.CurrentDataAddr = NVM_SECTOR_SIZE;
    lNvm.MemoryStatus = 0u;
    
    /* Mark nvm based data as dirty */
    for (i=0; i<NVM_NUMBER_OF_BLOCKS; i++)
    {
        lNvm.CurrentBlockInstance[i] = 0u;
        
        lNvm.LastHeadAddr[i]= 0xFFFF;
        
        if (  (lNvm.BlockStatus[i]==NVM_BLOCKSTATE_NVM)
            ||(lNvm.BlockStatus[i]==NVM_BLOCKSTATE_WR_MAIN) )
        {
            lNvm.BlockStatus[i]=NVM_BLOCKSTATE_DIRTY;
        }
    }

    lNvm.InternalState = NVM_ST_REPAIR;
}

#ifdef NVM_READ_BEFORE_WRITE
static void local_NvmReadBeforeWrite(uint16 address, uint16 count)
{
    lNvm.CheckCount = count;
    Nvm_ReadMemory(address, count, lNvm.Buffer);
}

static tResult local_NvmCheckErased(void)
{
    tResult retval = RES_OK;
    uint16 i;
    
    for (i=0; i<lNvm.CheckCount; i++)
        if (lNvm.Buffer[i] != NVM_ERASE_VAL)
            retval = RES_INVALID;

    return retval;
}
#endif

static uint8 local_calc_CRC8(uint8 *buffer)
{
    return CalcCRC8( lNvm.CurrentHeader.InstanceNr+(lNvm.CurrentHeader.BlockSegAddr)+lNvm.CurrentHeader.BlockID,
                     buffer, 
                     cNvmBlockConfig[lNvm.CurrentHeader.BlockID].BlockSize); 
}