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

#define NVM_NUMBER_OF_BLOCKS    2

typedef enum
{
    NVDATA_ST_ERROR  = 0u,      /* Flash memory unavailable - default init state */
    NVDATA_ST_INIT,             /* Initialization */
    NVDATA_ST_INITDATA,         /* Initialization of NVM Data */
    NVDATA_ST_IDLE              /* Nothing to do */
    
} tNvmDataInternalStates;

typedef struct
{
    uint16      BlockSize;
}tNvmBlockStructure;

typedef struct
{
    uint16  CurrentInstance;
    uint16  CurrentBlockAddress;
} tNvmBlockData;
     
typedef struct
{
    tNvmBlockData           CurrentBlockData[NVM_NUMBER_OF_BLOCKS];
    tNvmDataInternalStates  InternalState;
    uint16                  CurrentHeadAddr;
    uint16                  CurrentDataAddr;
    uint8                   CurrentSector;
    
} tRtAppDataStruct;

/*--------------------------------------------------
 *                  Local Variables and prototypes
 *--------------------------------------------------*/

static tRtAppDataStruct  lRtAppData;

#ifdef APPDATA_NVM_USE_RAM
static uint8 lRtAppDataRamBuffer[0x4000]; /* 4 x 4K flash sector emulated in RAM */
#endif

/*  Flash handling funtions */
inline tCypFlashStatus local_GetMemoryStatus(void);
void local_ReadBulkMemory(uint32 address, uint16 count, uint8* buffer);

/* NVM internal functions */
static void local_ReadNVMDataBlock(uint8 blockID, uint16 address);

/* Storage Memory internal functions */

/*--------------------------------------------------
 *                  Interface Functions
 *--------------------------------------------------*/

void RtAppData_Init(void)
{
    PORT_PIN_LED_ON_ON();
    /* initialize local variables */
    memset(&lRtAppData, 0, sizeof(tRtAppDataStruct));

#ifdef APPDATA_NVM_USE_RAM
    memset(&lRtAppDataRamBuffer, 0xFF, 0x4000);
    lRtAppData.InternalState = NVDATA_ST_INIT;
#else
    if (CypFlash_GetStatus() == CYPFLASH_ST_READY)
    {
        lRtAppData.InternalState = NVDATA_ST_INIT;
    }
#endif

    local_ReadNVMDataBlock(0,0);

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
            lRtAppData.InternalState = NVDATA_ST_INITDATA;
            break;
        }
    case NVDATA_ST_INITDATA:
        {
            local_ReadNVMDataBlock(0,0);
            PORT_PIN_LED_ON_OFF();
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
inline tCypFlashStatus local_GetMemoryStatus(void)
{
#ifdef APPDATA_NVM_USE_RAM
    return CYPFLASH_ST_READY;
#else
    return CypFlash_GetStatus();
#endif
}

void local_ReadBulkMemory(uint32 address, uint16 count, uint8* buffer)
{
#ifdef APPDATA_NVM_USE_RAM
    memcpy(&buffer, lRtAppDataRamBuffer, count);
#else
    if (CypFlash_Read(address) != CYPFLASH_ST_READY)
    {
        lRtAppData.InternalState = NVDATA_ST_ERROR;
    }
#endif
}

static void local_ReadNVMDataBlock(uint8 blockID, uint16 address)
{

}
