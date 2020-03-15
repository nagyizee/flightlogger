#include "ExampleRtApp.h"
#include "HALport.h"
#include "Os.h"

//TODO: remove the whole module from project scope after tests are done

#include "HALI2c.h"
#include "HALSpi.h"
#include "CypFlash.h"

/* Test control and data variables */

#define LONGBUFF_SIZE       (4096)

static uint32 tasktimingtest = 0;

static uint32 i2ctest = 0;
static tI2CChannelType i2ctest_ch = HALI2C_CHANNEL_BAROMETER;
static uint32 i2ctest_tx_size = 1;
static uint32 i2ctest_rx_size = 1;
static uint8 i2ctest_reg;

static uint32 spitest = 0;
static uint32 spitest_filltype = 1;
static uint16 spitest_tx_size = 40;
static uint16 spitest_rx_size = 1;
static uint8  spitest_padding = 0x01;

uint8 buff_1[LONGBUFF_SIZE];
uint8 buff_2[LONGBUFF_SIZE];


static uint32 cypflashtest = 0; 
/* Possible values: 0 - inactive, 1 - read, 2 - write, 3 - page write, 4 - erase sector, 5 - erase block */
uint8 flash_buff[256];
static uint32 baseaddress = 0;

static void local_i2c_test(void);
static void local_spi_test(void);
static void local_fillbuffer(uint8 *buffer);

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

    local_i2c_test();
    local_spi_test();

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



static void local_i2c_test(void)
{
    volatile uint32 ctr = 0;

    if (i2ctest == 0)
    {
        return;
    }

    switch (i2ctest)
    {
        case 1:
            HALI2C_Write(i2ctest_ch, buff_1, i2ctest_tx_size);
            break;
        case 2:
            HALI2C_Read(i2ctest_ch, buff_1, i2ctest_rx_size);
            break;
        case 3:
            HALI2C_ReadRegister(i2ctest_ch, i2ctest_reg, buff_1, i2ctest_rx_size);
            break;
      }

    while (HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER) == I2C_BUSY)
    {
        ctr++;
    }

    i2ctest = 0;
}


static void local_spi_test(void)
{
    volatile uint32 ctr = 0;
    volatile tResult res;

    if (spitest == 0)
    {
        return;
    }

    switch (spitest)
    {
        case 1:
            res = HALSPI_SetCS(0);
            break;
        case 2:
            HALSPI_ReleaseCS(0);
            break;
        case 3:
            res = HALSPI_TxData(0, spitest_tx_size, buff_1);
            break;
        case 4:
            res = HALSPI_RxData(0, spitest_rx_size, buff_2);
            break;
        case 5:
            res = HALSPI_StartTransfer(0);
            break;
        case 6:
            local_fillbuffer(buff_1);
            break;
        case 7:
            local_fillbuffer(buff_2);
            break;
        case 8:
            local_fillbuffer(buff_1);
            HALSPI_SetCS(0);
            HALSPI_TxData(0, spitest_tx_size, buff_1);
            HALSPI_StartTransfer(0);
            break;
        case 9:
            local_fillbuffer(buff_1);
            local_fillbuffer(buff_2);
            HALSPI_SetCS(0);
            HALSPI_TxData(0, spitest_tx_size, buff_1);
            HALSPI_RxData(0, spitest_rx_size, buff_2);
            HALSPI_StartTransfer(0);
            break;

    }

    while (HALSPI_GetStatus(0) == SPI_BUSY)
    {
        ctr++;
    }

}

static void local_fillbuffer(uint8 *buffer)
{
    uint32 i;

    switch (spitest_filltype)
    {
        case 0: /* fill with padding byte */
            for (i = 0; i < LONGBUFF_SIZE; i++)
            {
                buffer[i] = spitest_padding;
            }
            break;
        case 1: /* fill with increments */
            for (i = 0; i < LONGBUFF_SIZE; i++)
            {
                buffer[i] = spitest_padding;
                spitest_padding++;
            }
            break;
    }


}
