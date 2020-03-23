#include "RtAppExample.h"

#ifdef RTAPPEXAMPLEACTIVE

#include "HALport.h"
#include "Os.h"

#include "HALI2c.h"
#include "HALSpi.h"
#include "CypFlash.h"
#include "Nvm.h"
#include "Nvm_Cfg.h"

/* Test control and data variables */

#define LONGBUFF_SIZE       (4096)

#define FLS_UNRESET()       do {PORT_PIN_FLS_RESET_ON();} while(0)     /* Signal 1 on RESET unresets the flash chip */
#define FLS_RESET()         do {PORT_PIN_FLS_RESET_OFF();} while(0)    /* Signal 0 on RESET resets the flash chip */

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
static tCypFlashStatus cypflashcallstatus;
static uint8 flash_buff[CYPFLASH_READ_BUFSIZE];
static uint32 baseaddress = 0;
static uint8 writecount = 1;
static uint8 writedata = 0xaa;

static uint32 nvmtest = 0;
static uint8 nvmtest_blockid = 0;
static uint16 nvmtest_blocksize = 8;
static tNvmBlockStatus nvmtest_retval;
static uint8 nvmtest_buff[NVM_MAX_BLOCK_SIZE];

static void local_tasktiming_test(uint32 taskIdx);
static void local_i2c_test(void);
static void local_spi_test(void);
static void local_cypflashtest(void);
static void local_fillbuffer(uint8 *buffer);

void RtAppExample_Main(uint32 taskIdx)
{
    local_tasktiming_test(taskIdx);
    local_i2c_test();
    local_spi_test();
    local_cypflashtest();
}

static void local_tasktiming_test(uint32 taskIdx)
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
        case 10:
            /* ext.flash comm. test - read ID: - one shot */
            FLS_UNRESET();
            buff_1[0] = 0x9F;
            HALSPI_TxData(0, 1, buff_1);
            HALSPI_RxData(0, 4, buff_2);
            HALSPI_SetCS(0);
            HALSPI_StartTransfer(0);
            while (HALSPI_GetStatus(0) == SPI_BUSY) {}
            HALSPI_ReleaseCS(0);
            /* buff_2 should be: xx 01 60 17 */
            break;
        case 11:
            /* ext.flash comm. test - read ID: - 2 separated commands */
            FLS_UNRESET();
            buff_1[0] = 0x9F;
            HALSPI_TxData(0, 1, buff_1);
            HALSPI_SetCS(0);
            HALSPI_StartTransfer(0);
            while (HALSPI_GetStatus(0) == SPI_BUSY) {}      /* wait for sending command */
            HALSPI_RxData(0, 3, buff_2);
            HALSPI_StartTransfer(0);
            while (HALSPI_GetStatus(0) == SPI_BUSY) {}      /* wait for receiving data */
            HALSPI_ReleaseCS(0);
            /* buff_2 should be: 01 60 17 */
            break;
    }

    while (HALSPI_GetStatus(0) == SPI_BUSY)
    {
        ctr++;
    }

    spitest = 0;
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

static void local_cypflashtest(void)
{
    if (cypflashtest)
    {
        uint16 i;
        
        switch(cypflashtest)
        {
        case 1: /* Read */
            {
                cypflashcallstatus = CypFlash_Read(baseaddress, CYPFLASH_READ_BUFSIZE, &flash_buff[0]);
                break;
            }
        case 2: /* Write 64 bytes from address */
            {
                for (i=0; i<writecount; i++) flash_buff[i] = writedata;
                cypflashcallstatus = CypFlash_Write(baseaddress, writecount, &flash_buff[0]);
                break;
            }
        case 3: /* Page write with 0x55 */
            {
                for (i=0; i<CYPFLASH_WRITE_BUFSIZE; i++) flash_buff[i] = writedata;
                cypflashcallstatus = CypFlash_WritePage(baseaddress, &flash_buff[0]);
                break;
            }
        case 4: /* Erase 4KB sector */
            {
                cypflashcallstatus = CypFlash_EraseSector(baseaddress);
                break;
            }
        case 5: /* Erase 64 KB block */
            {
                cypflashcallstatus = CypFlash_EraseBlock(baseaddress);
                break;
            }
        case 6: /* Erase entire chip */
            {
                cypflashcallstatus = CypFlash_EraseAll();
                break;
            }
        default: /* Wrong test code */
            {
            }
        }
        cypflashtest=0;
        
        if (cypflashcallstatus != CYPFLASH_ST_READY)
        {
            /* Flash was busy at the time of the command */
            baseaddress = CypFlash_GetStatus();
        }
    }
}

static void local_nvm_test(void)
{
    switch(nvmtest)
    {
    case 1:     /* Read block */
        {
            nvmtest_retval = Nvm_ReadBlock(nvmtest_blockid, nvmtest_buff, nvmtest_blocksize);
            nvmtest = 0;
            break;
        }
    case 2:     /* Write block */
        {
            nvmtest_retval = Nvm_WriteBlock(nvmtest_blockid, nvmtest_buff, nvmtest_blocksize);
            nvmtest = 0;
            break;
        }
    case 3:     /* Continuous write */
        {
            nvmtest_retval = Nvm_WriteBlock(nvmtest_blockid, nvmtest_buff, nvmtest_blocksize);
            if (nvmtest_retval != NVM_MEMORY_FULL)
                nvmtest_buff[1]++;
            else
                nvmtest = 0;
                
            break;
        }
    case 4:
        {
            Nvm_PurgeHistory();
        }
    default: /* Wrong test code */
        {
            nvmtest = 0;
        }
    }
    
}
#endif