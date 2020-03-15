/* Cypress S25FL064 External flash driver - using standard SPI interface */

/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include <string.h>
#include "base.h"
#include "CypFlash.h"
#include "HALSpi.h"

/*--------------------------------------------------
 *                  Defines and type definitions
 *--------------------------------------------------*/

#define FLS_UNRESET()       do {PORT_PIN_FLS_RESET_ON();} while(0)     /* Signal 1 on RESET unresets the flash chip */
#define FLS_RESET()         do {PORT_PIN_FLS_RESET_OFF();} while(0)    /* Signal 0 on RESET resets the flash chip */

#define FLASH_MAX_ADDRESS   16*1024*1024 /* 16 MB flash size */
#define FLASH_SPI_ID        0

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
    tCypFlashDeviceStatus DeviceStatus;
    tCypFlashStatus       Status;
    uint8                 Tx_Buffer[260];
    uint8                 Rx_Buffer[260];
} tCypFlashStruct;

/*--------------------------------------------------
 *         Local Variables and prototypes
 *--------------------------------------------------*/

static tCypFlashStruct  lCypFlash;

/* Device handling functions */
static void local_CypFlash_GetDeviceStatus(void);

/*--------------------------------------------------
 *             Interface Functions
 *--------------------------------------------------*/

/* Generic driver interfaces */
void CypFlash_Init(void)
{
    /* initialize local variables */
    memset(&lCypFlash, 0, sizeof(tCypFlashStruct));
}

void CypFlash_MainFunction(void)
{
    switch (lCypFlash.Status)
    {
    case CYPFLASH_ST_UNINITIALIZED:
        {
            tResult lres;
            FLS_UNRESET();
            /* Get and check device ID to verify proper communication */
            lCypFlash.Tx_Buffer[0] = 0x9F;
            HALSPI_TxData(FLASH_SPI_ID, 1, lCypFlash.Tx_Buffer);
            HALSPI_RxData(FLASH_SPI_ID, 4, lCypFlash.Rx_Buffer);
            lres = HALSPI_SetCS(FLASH_SPI_ID);
            if (lres == RES_OK)
            {
                HALSPI_StartTransfer(FLASH_SPI_ID);   
                while (HALSPI_GetStatus(FLASH_SPI_ID) == SPI_BUSY) {}
                HALSPI_ReleaseCS(FLASH_SPI_ID);
                /* result should be: xx 01 60 17 */
                if ((lCypFlash.Rx_Buffer[1]==0x01)&&(lCypFlash.Rx_Buffer[2]==0x60)&&(lCypFlash.Rx_Buffer[3]==0x17))
                {
                    lCypFlash.Status = CYPFLASH_ST_READY;
                }
                else
                {
                    lCypFlash.Status = CYPFLASH_ST_COMM_ERROR;
                }
            }
            else
            {
                /* SPI driver busy - comm failure */
                lCypFlash.Status = CYPFLASH_ST_COMM_ERROR;
            }
            break;
        }
    case CYPFLASH_ST_READY:
        {
            break;
        }
    case CYPFLASH_ST_BUSY:
        {
            break;
        }
    case CYPFLASH_ST_PARAM_ERROR:
        {
            break;
        }
    case CYPFLASH_ST_COMM_ERROR:
        {
            break;
        }
    case CYPFLASH_ST_DEVICE_ERROR:
        {
            break;
        }
    default: while(1); //Force WDT reset
    }
}

tCypFlashStatus CypFlash_GetStatus(void)
{
    return lCypFlash.Status;
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
tCypFlashStatus CypFlash_Read(uint32 address, uint16 count, uint8* buffer)
{
    static tCypFlashStatus l_return_status;
    l_return_status = lCypFlash.Status;
        
    /* Check input parameters */
    if ((address+count < FLASH_MAX_ADDRESS)&&((count>0)&&(buffer!=NULL)))
    {
        
    }
    else
    {
        l_return_status = CYPFLASH_ST_PARAM_ERROR;
    }
      return l_return_status;
}

tCypFlashStatus CypFlash_Write(uint32 address, uint16 count, uint8* buffer)
{
    PORT_PIN_LED_ON_ON();
    PORT_PIN_LED_BLE_OFF();
    return lCypFlash.Status;
}

tCypFlashStatus CypFlash_WritePage(uint32 address, uint8* buffer)
{
    PORT_PIN_LED_ON_ON();
    PORT_PIN_LED_BLE_ON();
    return lCypFlash.Status;
}

tCypFlashStatus CypFlash_EraseSector(uint32 address) /* 4K sector */
{
    PORT_PIN_LED_ON_OFF();
    PORT_PIN_LED_BLE_ON();
    return lCypFlash.Status;
}

tCypFlashStatus CypFlash_EraseBlock(uint32 address) /* 32K block */
{
    PORT_PIN_LED_ON_OFF();
    PORT_PIN_LED_BLE_ON();
    return lCypFlash.Status;
}

/*--------------------------------------------------
 *             Local functions
 *--------------------------------------------------*/

static void local_CypFlash_GetDeviceStatus(void)
{
    /* Get the HW status register from the device */
  
}