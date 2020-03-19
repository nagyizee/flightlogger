/* NVM and Storage Memory Manager */

/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include <string.h>
#include "base.h"
#include "HALPort.h"
#include "CypFlash.h"
#include "RtAppData.h"

/*--------------------------------------------------
 *                  Defines and type definitions
 *--------------------------------------------------*/
//#define APPDATA_NVM_USE_RAM  /* Comment to use CypFlash insted of RAM emulated NVM */

#define NVM_NUMBER_OF_BLOCKS         3
#define NVM_HEADER_SIZE              8  /* Bytes */
#define NVM_MAX_BLOCK_SIZE          16  /* Bytes */
#define NVM_TOTAL_SIZE_OF_BLOCKS    32  /* Only data blocks, without header info! */

#define NVM_BLOCKSTATE_DEFAULT      0
#define NVM_BLOCKSTATE_NVM          1
#define NVM_BLOCKSTATE_DIRTY        2
#define NVM_BLOCKSTATE_WR_MAIN      3   /* Written to the main sector, writing to mirror still ongoing */

typedef enum
{
    NVDATA_ST_INIT = 0,         /* Initialization */
    NVDATA_ST_INITHEADER,       /* Initialization of NVM Header */
    NVDATA_ST_INITDATA,         /* Initialization of NVM Data */
    NVDATA_ST_IDLE,             /* Nothing to do */
    NVDATA_ST_REPAIR,           /* Erase all sectors and rebuild nvm */
    NVDATA_ST_REPAIR_CNT,       /*      Continue */
    NVDATA_ST_ERROR             /* Flash memory unavailable */    
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
    uint16      BlockAddress;   /* Start address of the block in the NVM array */
    uint8       BlockSize;
    uint8       MagicWord;
}tNvmBlockStruct;
     
typedef struct
{
    tNvmBlockStruct         ConfigMirror[NVM_NUMBER_OF_BLOCKS];
    uint16                  CurrentBlockInstance[NVM_NUMBER_OF_BLOCKS];
    uint8                   RamMirror[NVM_TOTAL_SIZE_OF_BLOCKS];
    uint8                   Buffer[NVM_MAX_BLOCK_SIZE];
    tNvmBlockHeaderStruct   CurrentHeader;
    uint16                  CurrentHeadAddr;
    uint16                  CurrentDataAddr;
    uint8                   CurrentSector;
    tNvmDataInternalStates  InternalState;
    uint8                   BlockFound;
    uint8                   BlockState[NVM_NUMBER_OF_BLOCKS]; /* 0 - default, 1 - nv data, 2 - dirty */
} tRtAppDataStruct;

/*--------------------------------------------------
 *                  Local Variables and prototypes
 *--------------------------------------------------*/

static const tNvmBlockStruct cNvmBlockConfig[NVM_NUMBER_OF_BLOCKS] = 
{   /*  BlockAddress    BlockSize   MagicWord*/
    {   {0x0000},       { 8},       {0x1D}},       /* Block 0 - Device identification */
    {   {0x0008},       { 8},       {0xEE}},       /* Block 1 - Device Health status */
    {   {0x0010},       {16},       {0xCE}}        /* Block 2 - Comm/Flight Event */
};

static const uint8 cNvmDefaultData[NVM_TOTAL_SIZE_OF_BLOCKS] =
{   /* Block 0 - Device identification - size 8 bytes */
    /* Magic    HW Ver  SW Version  Unique ID   UID Inverted */
        0x1D,   0x01,   0x00,0x01,  0xFF,0xFF,   0xFF,0xFF, 

    /* Block 1 - Device Health status - size 8 bytes */
    /* Magic        DateTimestamp     Error code + data    */
        0xEE,   0xFF,0xFF,0xFF,0xFF,    0xFF,   0xFF,0xFF,
        
    /* Block 2 Comm Event */
    /* Magic    Date        Time        JudgeID     EventCode    */
        0xCE,   0xFF,0xFF,  0xFF,0xFF,  0xFF,0xFF,    0xFF,
            /*  USERid      AirplaneID  CategoryID      reserved */
                0xFF,0xFF,  0xFF,0xFF,  0xFF,       0xFF,0xFF,0xFF
};

static tRtAppDataStruct  lRtAppData;

#ifdef APPDATA_NVM_USE_RAM
static uint8 lRtAppDataRamBuffer[0x4000]; /* 4 x 4K flash sector emulated in RAM */
#endif

/*  Flash handling funtions */
static tCypFlashStatus local_GetMemoryStatus(void);
static tCypFlashStatus local_StartEraseSector(uint32 address);
static void local_ReadMemory(uint32 address, uint16 count, uint8* buffer);

/* NVM internal functions */
static void local_NvmHandleCorruptMemory(void);
static void local_NvmStartRepairProcess(void);

/* Storage Memory internal functions */

/* CRC */
uint16 CalcCRC16(uint16 startval, uint8* buffer, uint16 counter);
/*--------------------------------------------------
 *                  Interface Functions
 *--------------------------------------------------*/

void RtAppData_Init(void)
{
    PORT_PIN_LED_ON_ON();
    /* initialize local variables */
    memset(&lRtAppData, 0, sizeof(tRtAppDataStruct));
    memcpy(&lRtAppData.ConfigMirror, &cNvmBlockConfig, sizeof(cNvmBlockConfig));
    /* Fill RAM mirror with default data */
    memcpy(&lRtAppData.RamMirror, &cNvmDefaultData, sizeof(cNvmDefaultData));
    lRtAppData.CurrentDataAddr = 0x1000;
    
    /* Check memory source availability */
#ifdef APPDATA_NVM_USE_RAM
    memset(lRtAppDataRamBuffer, 0xFF, 0x4000);
#else
    if (CypFlash_GetStatus() != CYPFLASH_ST_READY)
    {
        lRtAppData.InternalState = NVDATA_ST_ERROR;
    }
#endif

    while(lRtAppData.InternalState < NVDATA_ST_IDLE) RtAppData_Main();
}

void RtAppData_Main(void)
{
    switch(lRtAppData.InternalState)
    {   
    case NVDATA_ST_IDLE:
        {
            break;
        }
    case NVDATA_ST_INIT:
        {
            /* Parse header areas - start with Sector 0 */
            /* Start to check actual sector */
            local_ReadMemory(lRtAppData.CurrentHeadAddr, NVM_HEADER_SIZE, lRtAppData.Buffer);

            lRtAppData.InternalState = NVDATA_ST_INITHEADER;
            break;
        }
    case NVDATA_ST_INITHEADER:
        {
            if (local_GetMemoryStatus() == CYPFLASH_ST_READY)
            {
                switch (lRtAppData.Buffer[0])
                {
                case 0x0A:  /* History data considered valid until newer data will be found */
                case 0xAA:  /* Valid header, save and process header information */
                    {                        
                        memcpy(&lRtAppData.CurrentHeader, lRtAppData.Buffer, NVM_HEADER_SIZE);
                        
                        /* Check plausibility of block ID and address  */
                        if (((lRtAppData.CurrentHeader.BlockAbsAddress & 0xF000) == (lRtAppData.CurrentHeadAddr & 0xF000))
                            && (lRtAppData.CurrentHeader.BlockID < NVM_NUMBER_OF_BLOCKS))
                        {       /* read out data associated with actual header */
                            local_ReadMemory(lRtAppData.CurrentHeader.BlockAbsAddress, 
                                             lRtAppData.ConfigMirror[lRtAppData.CurrentHeader.BlockID].BlockSize, 
                                             lRtAppData.Buffer);
                            lRtAppData.InternalState = NVDATA_ST_INITDATA;
                        }
                        else
                        {       /* Invalid address within header */
                           local_NvmHandleCorruptMemory();
                        }
                        break;
                    }
                case 0x55:  /* NVM continues in next sector */
                    {
                        memcpy(&lRtAppData.CurrentHeader, lRtAppData.Buffer, NVM_HEADER_SIZE);
                        
                        if ( ((lRtAppData.CurrentSector==0)||(lRtAppData.CurrentSector==2))
                            &&(lRtAppData.CurrentHeader.BlockID == 0xA5)
                            &&(lRtAppData.CurrentHeader.InstanceNr == 0xCAFE)
                            &&(lRtAppData.CurrentHeader.BlockAbsAddress == 0xDADA)
                            &&(lRtAppData.CurrentHeader.CRC16 == 0x0CBC)
                           )
                        {
                            lRtAppData.CurrentSector++;
                            lRtAppData.CurrentHeadAddr = lRtAppData.CurrentSector*0x1000;
                            lRtAppData.CurrentDataAddr = (lRtAppData.CurrentSector+1)*0x1000;
                            lRtAppData.InternalState = NVDATA_ST_INIT;
                        }
                        else
                        {   /* Invalid header data in the actual sector - header space corrupted, continue with mirror data */
                            local_NvmHandleCorruptMemory();
                        }
                        break;
                    }
                case 0x00:  /* Invalidated header ignored, go to next header */
                    {
                        lRtAppData.CurrentHeadAddr += NVM_HEADER_SIZE;
                        lRtAppData.InternalState = NVDATA_ST_INIT;
                        break;
                    }
                case 0xFF:  /* No more headers available in the actual sector, data OK */
                    {
                        if (lRtAppData.BlockFound>0) /* We already found data, finish */
                        {
                            PORT_PIN_LED_ON_OFF();
                            lRtAppData.InternalState = NVDATA_ST_IDLE;
                        }
                        else    /* Check other sectors if available */
                        {
                            if (lRtAppData.CurrentSector<3) /* Total of 4 sectors available for NVM - 2 operational + 2 mirrorred */
                            {
                                lRtAppData.CurrentSector++;
                                lRtAppData.CurrentHeadAddr = lRtAppData.CurrentSector*0x1000;
                                lRtAppData.InternalState = NVDATA_ST_INIT;
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
    case NVDATA_ST_INITDATA:
        {
            if (local_GetMemoryStatus() == CYPFLASH_ST_READY)
            {   /* Check header and block data consistency */
                if (  (lRtAppData.Buffer[0]==lRtAppData.ConfigMirror[lRtAppData.CurrentHeader.BlockID].MagicWord)
                    &&(CalcCRC16(lRtAppData.CurrentHeader.InstanceNr+lRtAppData.CurrentHeader.BlockAbsAddress+lRtAppData.CurrentHeader.BlockID,
                                 lRtAppData.Buffer, 
                                 lRtAppData.ConfigMirror[lRtAppData.CurrentHeader.BlockID].BlockSize) 
                       == lRtAppData.CurrentHeader.CRC16)
                    )
                {
                    /* Data valid, store in RAM Mirror */
                    memcpy(lRtAppData.RamMirror+lRtAppData.ConfigMirror[lRtAppData.CurrentHeader.BlockID].BlockAddress,
                           lRtAppData.Buffer,
                           lRtAppData.ConfigMirror[lRtAppData.CurrentHeader.BlockID].BlockSize);

                    /* Mark block state */
                    lRtAppData.BlockFound = 1;
                    lRtAppData.BlockState[lRtAppData.CurrentHeader.BlockID] = NVM_BLOCKSTATE_NVM;
                    
                    /* Update Instance number */
                    lRtAppData.CurrentBlockInstance[lRtAppData.CurrentHeader.BlockID] = lRtAppData.CurrentHeader.InstanceNr;
                    
                    /* Update global BlockAddress */
                    lRtAppData.CurrentDataAddr = lRtAppData.CurrentHeader.BlockAbsAddress;
                    
                    /* Parse next header */
                    lRtAppData.CurrentHeadAddr += NVM_HEADER_SIZE;
                    lRtAppData.InternalState = NVDATA_ST_INIT;
                }
                else        /* Data corrupted, go to mirror area */
                {
                    local_NvmHandleCorruptMemory();
                }
            }
            break;
        }
    case NVDATA_ST_REPAIR:
        {
            if (local_StartEraseSector(lRtAppData.CurrentHeadAddr) == CYPFLASH_ST_READY)
                lRtAppData.InternalState = NVDATA_ST_REPAIR_CNT;
            break;
        }
    case NVDATA_ST_REPAIR_CNT:
        {
            if (local_GetMemoryStatus() == CYPFLASH_ST_READY)
            {
                if (lRtAppData.CurrentSector<3) /* Total of 4 sectors available for NVM - 2 operational + 2 mirrorred */
                {
                    lRtAppData.CurrentSector++;
                    lRtAppData.CurrentHeadAddr = lRtAppData.CurrentSector*0x1000;
                    lRtAppData.InternalState = NVDATA_ST_REPAIR;
                }
                else
                {
                    lRtAppData.CurrentSector = 0;
                    lRtAppData.CurrentHeadAddr = 0;
                    lRtAppData.InternalState = NVDATA_ST_IDLE;
                }
            }
            break;
        }
    case NVDATA_ST_ERROR:
        {   /* Error case - erase memory and rebuild data */
            
/* !!!!!!!!!!!!!!!!!!!!!!!!! to be removed after os interrupts will be fixed */
            if(local_GetMemoryStatus() == CYPFLASH_ST_READY) lRtAppData.InternalState = NVDATA_ST_INIT;
            break;
        }
    default:
        {
            /* Error case */
            lRtAppData.InternalState = NVDATA_ST_ERROR;
        }
    }
}

/*--------------------------------------------------
 *             Local functions
 *--------------------------------------------------*/
static tCypFlashStatus local_GetMemoryStatus(void)
{
#ifdef APPDATA_NVM_USE_RAM
    return CYPFLASH_ST_READY;
#else
    return CypFlash_GetStatus();
#endif
}

static tCypFlashStatus local_StartEraseSector(uint32 address)
{
#ifdef APPDATA_NVM_USE_RAM
    memset(lRtAppDataRamBuffer+address, 0xFF, 0x1000);
    return CYPFLASH_ST_READY;
#else
    return CypFlash_EraseSector(address);
#endif
}

static void local_ReadMemory(uint32 address, uint16 count, uint8* buffer)
{
#ifdef APPDATA_NVM_USE_RAM
    memcpy(buffer, lRtAppDataRamBuffer+address, count);
#else
    if (CypFlash_Read(address, count, buffer) != CYPFLASH_ST_READY)
    {
        lRtAppData.InternalState = NVDATA_ST_ERROR;
    }
#endif
}

static void local_NvmHandleCorruptMemory(void)
{
    /* Invalid header data in the actual sector - header space corrupted, continue with mirror data */
    /* If current sector < 2, check the same mirror header area */
    if (lRtAppData.CurrentSector<2)
    {
            lRtAppData.CurrentSector+=2;
            lRtAppData.CurrentHeadAddr += 0x2000;
            lRtAppData.InternalState = NVDATA_ST_INIT;
    }
    else    /* Already in mirror flash space, start repair process */
    {
        local_NvmStartRepairProcess();
    }
}

static void local_NvmStartRepairProcess(void)
{
    uint32 i;
    
    lRtAppData.CurrentSector = 0u;
    lRtAppData.CurrentHeadAddr = 0u;
    lRtAppData.CurrentDataAddr = 0x1000;
    
    /* Mark nvm based data as dirty */
    for (i=0; i<NVM_NUMBER_OF_BLOCKS; i++)
    {
        lRtAppData.CurrentBlockInstance[i] = 0u;
        if (lRtAppData.BlockState[i]==NVM_BLOCKSTATE_NVM)
            lRtAppData.BlockState[i]=NVM_BLOCKSTATE_DIRTY;
    }

    lRtAppData.InternalState = NVDATA_ST_REPAIR;
}

uint16 CalcCRC16(uint16 startval, uint8* buffer, uint16 counter)
{
    uint16 acc = startval, i;
    for (i=0; i<counter; i++) acc += (buffer[i]+i)*buffer[i];
    //return acc;
    return 0xFFFF;
}