/* Cypress S25FL064 External flash driver - using standard SPI interface */

/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include "base.h"
#include "CypFlash.h"
#include "HALSpi.h"

/*--------------------------------------------------
 *                  Defines and type definitions
 *--------------------------------------------------*/

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

typedef struct
{
    tCypFlashStatus       CypFlash_Status;
    tCypFlashDeviceStatus CypFlashDeviceStatus;
} tCypFlashStruct;

/*--------------------------------------------------
 *         Local Variables and prototypes
 *--------------------------------------------------*/

static tCypFlashStruct  lCypFlash = 
{ 
    CYPFLASH_ST_UNINITIALIZED,
    0x0000u,
};

/* Device handling functions */
static void local_CypFlash_GetDeviceStatus(void);

/*--------------------------------------------------
 *             Interface Functions
 *--------------------------------------------------*/

/* Generic driver interfaces */
void CypFlash_Init(void)
{

}

void CypFlash_MainFunction(void)
{
    local_CypFlash_GetDeviceStatus();
}

tCypFlashStatus CypFlash_GetStatus(void)
{
    return lCypFlash.CypFlash_Status;
}

/* Device handling functions */
void CypFlash_Sleep(void)
{
  
}

void CypFlash_Wake(void)
{
}

void CypFlash_EraseAll(void)
{
}

/*  Data handling functions */
tCypFlashStatus CypFlash_Read(void)
{
    return lCypFlash.CypFlash_Status;
}

tCypFlashStatus CypFlash_Write(void)
{
    return lCypFlash.CypFlash_Status;
}

tCypFlashStatus CypFlash_WritePage(void)
{
    return lCypFlash.CypFlash_Status;
}

tCypFlashStatus CypFlash_EraseSector(void) /* 4K sector */
{
    return lCypFlash.CypFlash_Status;
}

tCypFlashStatus CypFlash_EraseBlock(void) /* 32K block */
{
    return lCypFlash.CypFlash_Status;
}

/*--------------------------------------------------
 *             Local functions
 *--------------------------------------------------*/

static void local_CypFlash_GetDeviceStatus(void)
{
    /* Get the HW status register from the device */
  
}