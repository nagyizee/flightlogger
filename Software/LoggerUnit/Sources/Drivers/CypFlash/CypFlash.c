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

#define FLASH_MAX_ADDRESS   16*1024*1024+1 /* 16 MB flash size */
#define FLASH_SPI_ID        0
#define RECOVER_RETRY_COUNT 5
#define RECOVER_WAIT_CYCLES 200 /* 1 second based on 5ms task cycle time */

#define FLS_CMD_GET_ID          0x9F
#define FLS_CMD_READ            0x03
#define FLS_CMD_WRITE           0x02
#define FLS_CMD_WRITE_ENABLE    0x06
#define FLS_CMD_ERASESECTOR     0x20
#define FLS_CMD_ERASEBLOCK      0xD8
#define FLS_CMD_READSTATUS1     0x05     

typedef struct
{
    uint8*                  ActualBuffer;
    uint16                  ActualCount;
    tCypFlashStatus         Status;
    uint8                   Tx_Buffer[260];
    uint8                   Rx_Buffer[260];
    uint8                   RecoverCounter;
    uint8                   RecoverIteration;
    uint8                   ActualCommand;
} tCypFlashStruct;

/*--------------------------------------------------
 *         Local Variables and prototypes
 *--------------------------------------------------*/

static tCypFlashStruct  lCypFlash;

/* Device handling functions */
static tCypFlashStatus local_CypFlash_StartErase(uint32 address);
static void local_CypFlash_GetDeviceStatus(void);
static void local_CypFlash_CheckDeviceStatus(volatile uint8 status);

/*--------------------------------------------------
 *             Interface Functions
 *--------------------------------------------------*/

/* Generic driver interfaces */
void CypFlash_Init(void)
{
    /* initialize local variables */
    memset(&lCypFlash, 0, sizeof(tCypFlashStruct));
    /* Force device initialisation phase */
    while(lCypFlash.Status<=CYPFLASH_ST_INITIALIZING) CypFlash_MainFunction();
}

void CypFlash_MainFunction(void)
{
    switch (lCypFlash.Status)
    {
    case CYPFLASH_ST_READY:
        {
            /* Nothing to do, quickly exit cyclic function */
            break;
        }
    case CYPFLASH_ST_UNINITIALIZED:
        {
            tResult lres;
            FLS_UNRESET();
            PORT_PIN_LED_BLE_ON();
            /* Get and check device ID to verify proper communication */
            lCypFlash.Tx_Buffer[0] = FLS_CMD_GET_ID;
            HALSPI_TxData(FLASH_SPI_ID, 1, lCypFlash.Tx_Buffer);
            HALSPI_RxData(FLASH_SPI_ID, 4, lCypFlash.Rx_Buffer);
            lres = HALSPI_SetCS(FLASH_SPI_ID);
            if (lres == RES_OK)
            {
                HALSPI_StartTransfer(FLASH_SPI_ID);   
                lCypFlash.Status = CYPFLASH_ST_INITIALIZING;
                lCypFlash.RecoverCounter = 0;
            }
            else
            {
                /* SPI driver busy - comm failure */
                lCypFlash.Status = CYPFLASH_ST_COMM_ERROR;
            }
            break;
        }
    case CYPFLASH_ST_INITIALIZING:
        {
            if (HALSPI_GetStatus(FLASH_SPI_ID) == SPI_READY)
            {
                HALSPI_ReleaseCS(FLASH_SPI_ID);
                /* result should be: xx 01 60 17 */
                if ((lCypFlash.Rx_Buffer[1]==0x01)&&
                    (lCypFlash.Rx_Buffer[2]==0x60)&&
                    (lCypFlash.Rx_Buffer[3]==0x17))
                {
                    lCypFlash.Status = CYPFLASH_ST_READY;
                    PORT_PIN_LED_BLE_OFF();
                }
                else
                {
                    lCypFlash.Status = CYPFLASH_ST_COMM_ERROR;
                }
            }
            else /* SPI still busy */
            {
                /* SPI deadlock timeout handling */
                if (lCypFlash.RecoverCounter >= RECOVER_WAIT_CYCLES)
                {
                    HALSPI_ReleaseCS(FLASH_SPI_ID);
                    lCypFlash.Status = CYPFLASH_ST_COMM_ERROR;
/* !! Don't reset lCypFlash.RecoverCounter only in this case to avoid extra wait time */
                }
                else
                {
                    lCypFlash.RecoverCounter++;
                }
            }
        break;
        }
    case CYPFLASH_ST_BUSY:
        {
            tResult lres;
            
            if (HALSPI_GetStatus(FLASH_SPI_ID) == SPI_READY)
            {
                HALSPI_ReleaseCS(FLASH_SPI_ID);
                switch(lCypFlash.ActualCommand)
                {
                case FLS_CMD_READ:
                    {
                        lCypFlash.Status = CYPFLASH_ST_READY;
                        PORT_PIN_LED_BLE_OFF();
                        memcpy(lCypFlash.ActualBuffer, &lCypFlash.Rx_Buffer[4], lCypFlash.ActualCount);
                        break;
                    }
                case FLS_CMD_WRITE:
                    {
                        switch(lCypFlash.Tx_Buffer[0]) /* Just executed command */
                        {
                        case FLS_CMD_WRITE_ENABLE:
                            {
                                lCypFlash.Tx_Buffer[0] = FLS_CMD_WRITE;
                                HALSPI_TxData(FLASH_SPI_ID, lCypFlash.ActualCount+4, lCypFlash.Tx_Buffer);
            
                                lres = HALSPI_SetCS(FLASH_SPI_ID);
                                if (lres == RES_OK)
                                {
                                    HALSPI_StartTransfer(FLASH_SPI_ID); 
                                    lCypFlash.RecoverCounter = RECOVER_WAIT_CYCLES - 1; /* 5ms timeout */
                                }
                                else
                                {
                                    lCypFlash.Status = CYPFLASH_ST_COMM_ERROR;
                                }
                                break;
                            }
                        case FLS_CMD_WRITE:
                            {
                                local_CypFlash_GetDeviceStatus();
                                break;
                            }
                        case FLS_CMD_READSTATUS1: /* 10ms cycle time */
                            {
                                local_CypFlash_CheckDeviceStatus(lCypFlash.Rx_Buffer[1]);
                                break;
                            }

                        default:
                            {
                                while(1); /* Wrong memory contents for actual command, assert */
                            }
                        }
                        break;
                    }
                case FLS_CMD_ERASEBLOCK:
                case FLS_CMD_ERASESECTOR:
                    {
                        switch(lCypFlash.Tx_Buffer[0]) /* Just executed command */
                        {
                        case FLS_CMD_WRITE_ENABLE:
                            {
                                lCypFlash.Tx_Buffer[0] = lCypFlash.ActualCommand;
                                HALSPI_TxData(FLASH_SPI_ID, 4, lCypFlash.Tx_Buffer);
            
                                lres = HALSPI_SetCS(FLASH_SPI_ID);
                                if (lres == RES_OK)
                                {
                                    HALSPI_StartTransfer(FLASH_SPI_ID); 
                                    lCypFlash.RecoverCounter = RECOVER_WAIT_CYCLES - 1; /* 5ms timeout */
                                }
                                else
                                {
                                    lCypFlash.Status = CYPFLASH_ST_COMM_ERROR;
                                }
                                break;
                            }
                        case FLS_CMD_ERASEBLOCK:
                        case FLS_CMD_ERASESECTOR:
                            {
                                local_CypFlash_GetDeviceStatus();
                                break;
                            }
                        case FLS_CMD_READSTATUS1: /* 5ms cycle time */
                            {
                                local_CypFlash_CheckDeviceStatus(lCypFlash.Rx_Buffer[1]);
                                break;
                            }
                        default:
                            {
                                while(1); /* Wrong memory contents for actual command, assert */
                            }
                        }
                        break;
                    }
                default:
                    {
                        while(1); /* Wrong memory contents for actual command, assert */
                    }
                }
            }
            else /* SPI still busy */
            {
                /* SPI deadlock timeout handling */
                if (lCypFlash.RecoverCounter >= RECOVER_WAIT_CYCLES)
                {
                    HALSPI_ReleaseCS(FLASH_SPI_ID);
                    lCypFlash.Status = CYPFLASH_ST_COMM_ERROR;
/* !! Don't reset lCypFlash.RecoverCounter only in this case to avoid extra wait time */
                }
                else
                {
                    lCypFlash.RecoverCounter++;
                }
            }
            break;
        }
    case CYPFLASH_ST_COMM_ERROR:
        {
            if (lCypFlash.RecoverIteration<RECOVER_RETRY_COUNT)
            {
                if (lCypFlash.RecoverCounter>=RECOVER_WAIT_CYCLES)
                {
                    FLS_RESET();
                    lCypFlash.RecoverIteration++;
                    lCypFlash.Status = CYPFLASH_ST_UNINITIALIZED;
                    lCypFlash.RecoverCounter=0;
                }
                else
                {
                    lCypFlash.RecoverCounter++;
                }
            }
            else
            {
                /* Device blocked */
                lCypFlash.Status = CYPFLASH_ST_DEVICE_ERROR;
            }
            break;
        }
    case CYPFLASH_ST_DEVICE_ERROR:
        {
            /* Fatal error concerning the memory device */
            break;
        }
    case CYPFLASH_ST_PARAM_ERROR:
        {
            //break; /* Not valid as an internal state. Continue to default state */
        }
    default: while(1); // Assert memory overwrite fatal error
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
    tResult lres;
    static tCypFlashStatus l_return_status;
    l_return_status = lCypFlash.Status;
    
    if (l_return_status == CYPFLASH_ST_READY)
    {
        /* Check input parameters - read 0 bytes not allowed, useless */
        if (((address&0x00FFFFFF)+count < FLASH_MAX_ADDRESS)
            &&(count>0)
            &&(buffer!=NULL))
        {
            PORT_PIN_LED_BLE_ON();
            lCypFlash.ActualCommand = FLS_CMD_READ;
            lCypFlash.ActualCount = count;
            lCypFlash.ActualBuffer = buffer;
            
            lCypFlash.Tx_Buffer[0] = lCypFlash.ActualCommand;
            lCypFlash.Tx_Buffer[1] = (uint8)(address>>16);
            lCypFlash.Tx_Buffer[2] = (uint8)(address>>8);
            lCypFlash.Tx_Buffer[3] = (uint8)(address);
            
            /* Use double buffering for increased security */
            HALSPI_TxData(FLASH_SPI_ID, 4, lCypFlash.Tx_Buffer);
            HALSPI_RxData(FLASH_SPI_ID, count+4, lCypFlash.Rx_Buffer);
            
            lres = HALSPI_SetCS(FLASH_SPI_ID);
            if (lres == RES_OK)
            {
                HALSPI_StartTransfer(FLASH_SPI_ID); 
                lCypFlash.RecoverCounter = RECOVER_WAIT_CYCLES - 1; /* 5ms timeout */
                lCypFlash.Status = CYPFLASH_ST_BUSY;
                /* Return the previous READY state as everything going OK */
            }
            else
            {
                lCypFlash.Status = CYPFLASH_ST_COMM_ERROR;
                l_return_status = CYPFLASH_ST_COMM_ERROR;
            }
        }
        else
        {
            l_return_status = CYPFLASH_ST_PARAM_ERROR;
        }
    }
    return l_return_status;
}

tCypFlashStatus CypFlash_Write(uint32 address, uint16 count, uint8* buffer)
{
    tResult lres;
    static tCypFlashStatus l_return_status;
    l_return_status = lCypFlash.Status;
    
    if (l_return_status == CYPFLASH_ST_READY)
    {
        /* Check input parameters */
        if (((address&0x00FFFFFF)+count < FLASH_MAX_ADDRESS)
            &&(count>0)
            &&(buffer!=NULL))
        {
            PORT_PIN_LED_ON_ON();
            
            lCypFlash.ActualCommand = FLS_CMD_WRITE;
            lCypFlash.ActualCount = count;
            lCypFlash.ActualBuffer = buffer;
            
            lCypFlash.Tx_Buffer[0] = FLS_CMD_WRITE_ENABLE;
            lCypFlash.Tx_Buffer[1] = (uint8)(address>>16);
            lCypFlash.Tx_Buffer[2] = (uint8)(address>>8);
            lCypFlash.Tx_Buffer[3] = (uint8)(address);
            /* Use double buffering for increased security */
            memcpy(&lCypFlash.Tx_Buffer[4], buffer, count);
            
            HALSPI_TxData(FLASH_SPI_ID, 1, lCypFlash.Tx_Buffer);
            
            lres = HALSPI_SetCS(FLASH_SPI_ID);
            if (lres == RES_OK)
            {
                HALSPI_StartTransfer(FLASH_SPI_ID); 
                lCypFlash.RecoverCounter = RECOVER_WAIT_CYCLES - 1; /* 5ms timeout */
                lCypFlash.Status = CYPFLASH_ST_BUSY;
                /* Return the previous READY state as everything going OK */
            }
            else
            {
                lCypFlash.Status = CYPFLASH_ST_COMM_ERROR;
                l_return_status = CYPFLASH_ST_COMM_ERROR;
            }
        }
        else
        {
            l_return_status = CYPFLASH_ST_PARAM_ERROR;
        }
    }
    return l_return_status;
}

tCypFlashStatus CypFlash_WritePage(uint32 address, uint8* buffer)
{
    static tCypFlashStatus l_return_status;
    l_return_status = lCypFlash.Status;
    
    if (l_return_status == CYPFLASH_ST_READY)
    {
        if (address==(address&0x00FFFF00)) /* Check address alignment to page start*/
        {
            l_return_status = CypFlash_Write(address, 256, buffer);
        }
        else
        {
            l_return_status = CYPFLASH_ST_PARAM_ERROR;
        }
    }
    return l_return_status;
}

tCypFlashStatus CypFlash_EraseSector(uint32 address) /* 4K sector */
{
    static tCypFlashStatus l_return_status;
    l_return_status = lCypFlash.Status;
    
    if (l_return_status == CYPFLASH_ST_READY)
    {
        /* Check input parameters */
        if (address==(address&0x00FFF000)) /* Check address alignment to sector start*/
        {
            PORT_PIN_LED_ON_ON();
            PORT_PIN_LED_BLE_ON();
            
            lCypFlash.ActualCommand = FLS_CMD_ERASESECTOR;
            
            l_return_status = local_CypFlash_StartErase(address);
        }
        else
        {
            l_return_status = CYPFLASH_ST_PARAM_ERROR;
        }
    }
    return l_return_status;
}

tCypFlashStatus CypFlash_EraseBlock(uint32 address) /* 32K block */
{
    static tCypFlashStatus l_return_status;
    l_return_status = lCypFlash.Status;
    
    if (l_return_status == CYPFLASH_ST_READY)
    {
        /* Check input parameters */
        if (address==(address&0x00FF8000)) /* Check address alignment to sector start*/
        {
            PORT_PIN_LED_ON_ON();
            PORT_PIN_LED_BLE_ON();
            
            lCypFlash.ActualCommand = FLS_CMD_ERASEBLOCK;
            
            l_return_status = local_CypFlash_StartErase(address);
        }
        else
        {
            l_return_status = CYPFLASH_ST_PARAM_ERROR;
        }
    }
    return l_return_status;
}

/*--------------------------------------------------
 *             Local functions
 *--------------------------------------------------*/

static tCypFlashStatus local_CypFlash_StartErase(uint32 address)
{
    tResult lres;
    tCypFlashStatus l_return_status = CYPFLASH_ST_READY;
    
    lCypFlash.Tx_Buffer[0] = FLS_CMD_WRITE_ENABLE;
    lCypFlash.Tx_Buffer[1] = (uint8)(address>>16);
    lCypFlash.Tx_Buffer[2] = (uint8)(address>>8);
    lCypFlash.Tx_Buffer[3] = (uint8)(address);
    
    HALSPI_TxData(FLASH_SPI_ID, 1, lCypFlash.Tx_Buffer);
    
    lres = HALSPI_SetCS(FLASH_SPI_ID);
    if (lres == RES_OK)
    {
        HALSPI_StartTransfer(FLASH_SPI_ID); 
        lCypFlash.RecoverCounter = RECOVER_WAIT_CYCLES - 1; /* 5ms timeout */
        lCypFlash.Status = CYPFLASH_ST_BUSY;
        /* Return the previous READY state as everything going OK */
    }
    else
    {
        lCypFlash.Status = CYPFLASH_ST_COMM_ERROR;
        l_return_status = CYPFLASH_ST_COMM_ERROR;
    }
    return l_return_status;
}

static void local_CypFlash_GetDeviceStatus(void)
{
    tResult lres;

    lCypFlash.Tx_Buffer[0] = FLS_CMD_READSTATUS1;
    HALSPI_TxData(FLASH_SPI_ID, 1, lCypFlash.Tx_Buffer);
    HALSPI_RxData(FLASH_SPI_ID, 2, lCypFlash.Rx_Buffer);
    
    lres = HALSPI_SetCS(FLASH_SPI_ID);
    if (lres == RES_OK)
    {
        HALSPI_StartTransfer(FLASH_SPI_ID); 
        lCypFlash.RecoverCounter = 0; /* 1s timeout */
    }
    else
    {
        lCypFlash.Status = CYPFLASH_ST_COMM_ERROR;
    }
}

static void local_CypFlash_CheckDeviceStatus(volatile uint8 status)
{
    uint8 masked_status = status & 0x01;
    
    if (masked_status == 0) /* Device ready ? */
    {
        lCypFlash.Status = CYPFLASH_ST_READY;
        PORT_PIN_LED_ON_OFF();
        PORT_PIN_LED_BLE_OFF();
    }
    else
    {
        local_CypFlash_GetDeviceStatus();
    }
}
