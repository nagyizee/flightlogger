#include "ExampleRtApp.h"
#include "HALport.h"
#include "Os.h"

//TODO: remove the whole module from project scope after tests are done

#include "HALI2c.h"
#include "HALSpi.h"
#include "CypFlash.h"

/* Test control and data variables */
static uint32 tasktimingtest = 0;

static uint32 i2ctest = 0;
static tI2CChannelType ch = HALI2C_CHANNEL_BAROMETER;
static uint32 tx_size = 1;
static uint32 rx_size = 1;
static uint8 reg;
uint8 buff[16];

static uint32 spitest = 0;


static uint32 cypflashtest = 0; 
/* Possible values: 0 - inactive, 1 - read, 2 - write, 3 - page write, 4 - erase sector, 5 - erase block */
uint8 flash_buff[256];
static uint32 baseaddress = 0;

void ExampleRtApp_Main(uint32 taskIdx)
{

    if (tasktimingtest)
    {
        volatile uint32 count;
        for (count = 0; count < taskIdx; count++)
        {
            PORT_PIN_LED_ON_ON();
            asm("nop");
            asm("nop");
            PORT_PIN_LED_ON_OFF();
          }

        if (taskIdx)
        {
            count = 200;
        }
        else
        {
            count = 50;
        }

        PORT_PIN_LED_ON_ON();
        while (count)
        {
            count--;
        }

        if (taskIdx == 2)
        {
            Os_RtWakeUpBgndTask(2);
        }
        else if (taskIdx == 4)
        {
            Os_RtWakeUpBgndTask(4);
        }

        PORT_PIN_LED_ON_OFF();
    }

    if (i2ctest)
    {
        volatile uint32 ctr = 0;

        if (i2ctest == 1)
        {
            HALI2C_Write(ch, buff, tx_size);
        }
        else if (i2ctest == 2)
        {
            HALI2C_Read(ch, buff, rx_size);
        }
        else
        {
            HALI2C_ReadRegister(ch, reg, buff, rx_size);
        }

        while (HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER) == I2C_BUSY)
        {
            ctr++;
        }

        i2ctest = 0;
      }

    if (spitest)
    {
        static TSpiStatus ls_Spi_Status;
        
        ls_Spi_Status = HALSPI_SetCS(0);
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        HALSPI_ReleaseCS(0);
    }

    if (cypflashtest)
    {
        static tCypFlashStatus ls_CypFlash_Status;
        static uint16 i;
        
        switch(cypflashtest)
        {
        case 1: /* Read */
            {
                ls_CypFlash_Status = CypFlash_Read(baseaddress, 256, &flash_buff[0]);
                break;
            }
        case 2: /* write  */
            {
                for (i=0; i<64; i++) flash_buff[i] = i;
                ls_CypFlash_Status = CypFlash_Write(baseaddress, 64, &flash_buff[0]);
                break;
            }
        case 3: /* page write */
            {
                for (i=0; i<256; i++) flash_buff[i] = 0x55;
                ls_CypFlash_Status = CypFlash_WritePage(baseaddress, &flash_buff[0]);
                break;
            }
        case 4: /* erase sector */
            {
                ls_CypFlash_Status = CypFlash_EraseSector(baseaddress);
                break;
            }
        case 5: /* erase block */
            {
                ls_CypFlash_Status = CypFlash_EraseBlock(baseaddress);
                break;
            }
        default: 
            break;
        }
        cypflashtest=0;
      
    }

}
