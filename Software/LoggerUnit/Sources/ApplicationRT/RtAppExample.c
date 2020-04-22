#include "RtAppExample.h"

#ifdef RTAPPEXAMPLEACTIVE

#include "HALport.h"
#include "Os.h"

#include "HALI2c.h"
#include "HALSpi.h"
#include "CypFlash.h"
#include "Nvm.h"
#include "Nvm_Cfg.h"
#include "NxpBaro.h"
#include "NxpAccel.h"

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

static uint32 nxpbarotest = 0;
static uint32 nxpbarotest_mask = NXPBARO_ACQMASK_PRESSURE;
static int32 nxpbarotest_res_int;
static uint32 nxpbarotest_res_fract;


static uint32 i2c_barosensorcheck = 0;
static volatile uint32 i2c_accelsensorcheck = 0;
enum EPressOversampleRatio
{                           // minimum times between data samples:
    pos_none = 0x00,        // 6ms
    pos_2 = 0x08,           // 10ms
    pos_4 = 0x10,           // 18ms
    pos_8 = 0x18,           // 34ms
    pos_16 = 0x20,          // 66ms
    pos_32 = 0x28,          // 130ms
    pos_64 = 0x30,          // 258ms
    pos_128 = 0x38          // 512ms
};


static void local_tasktiming_test(uint32 taskIdx);
static void local_i2c_test(void);
static void local_spi_test(void);
static void local_cypflashtest(void);
static void local_fillbuffer(uint8 *buffer);
static void local_nvm_test(void);
static void local_nxp_baro_test(void);
static void local_i2c_baro_sensor_check(void);
static void local_i2c_accel_sensor_check(void);


void RtAppExample_Main(uint32 taskIdx)
{
    local_tasktiming_test(taskIdx);
    local_i2c_test();
    local_spi_test();
    local_cypflashtest();
    local_nvm_test();
    local_nxp_baro_test();
    local_i2c_baro_sensor_check();
    local_i2c_accel_sensor_check();
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

    while (HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER) != I2C_IDLE)
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

static void local_nxp_baro_test(void)
{
    volatile tNxpBaroStatus bstatus;
    volatile tResult res;
    uint32 retval;

    if (nxpbarotest == 0)
    {
        return;
    }

    switch(nxpbarotest)
    {
        case 1:
            bstatus = NXPBaro_GetStatus(&nxpbarotest_mask);
            nxpbarotest = 0;
            break;
        case 2:
            res = NXPBaro_Acquire(nxpbarotest_mask);
            nxpbarotest = 0;
            break;
        case 3:
            retval = NXPBaro_GetResult((tNxpBaroMeasurementSelector)nxpbarotest_mask);
            nxpbarotest = 0;
            break;
        case 4:
            NXPBaro_Acquire(nxpbarotest_mask);
            nxpbarotest = 5;
            break;
        case 5:
            bstatus = NXPBaro_GetStatus(&nxpbarotest_mask);
            if (bstatus == NXPBARO_ST_READY)
            {
                /* result ready */
                if (nxpbarotest_mask & NXPBARO_MS_ALTITUDE)
                {
                    retval = NXPBaro_GetResult(NXPBARO_MS_ALTITUDE);
                    /* conver to meters in float */
                    nxpbarotest_res_int = (int32)(retval >> 16) - NXPBARO_ALT_OFFSET;
                    nxpbarotest_res_fract = (retval & 0xFFFF) * 1000 / 0xFFFF;

                    nxpbarotest_mask &= ~NXPBARO_MS_ALTITUDE;
                }
                if (nxpbarotest_mask & NXPBARO_MS_PRESSURE)
                {
                    retval = NXPBaro_GetResult(NXPBARO_MS_PRESSURE);
                    /* conver to hPa in float */
                    nxpbarotest_res_int = (retval >> 8);
                    nxpbarotest_res_fract = (retval & 0xFF) * 1000 / 0xFF;

                    nxpbarotest_mask &= ~NXPBARO_MS_PRESSURE;
                }
                if (nxpbarotest_mask & NXPBARO_MS_TEMP)
                {
                    retval = NXPBaro_GetResult(NXPBARO_MS_TEMP);
                    /* conver to temperature in float */
                    nxpbarotest_res_int = (int32)(retval >> 16) - NXPBARO_TEMP_OFFSET;
                    nxpbarotest_res_fract = (retval & 0xFFFF) * 1000 / 0xFFFF;

                    nxpbarotest_mask &= ~NXPBARO_MS_TEMP;
                }

                if (nxpbarotest_mask == 0)
                {
                    nxpbarotest = 0;
                }
            }
            else if (bstatus != NXPBARO_ST_BUSY)
            {
                /* error or other stuff */
                nxpbarotest = 0;
            }
            break;
    }

}

static void local_i2c_baro_sensor_check(void)
{

#define REGPRESS_DATACFG        0x13            // 1byte Pressure data, Temperature data and event flag generator
#define REGPRESS_OUTP           0x01            // 3byte barometric data + 2byte thermometric data - pressure data is in Pascales - 20bit: 18.2 from MSB.
#define PREG_DATACFG_DREM       0x04            // data reay event mode
#define PREG_DATACFG_PDEFE      0x02            // event detection for new pressure data
#define PREG_DATACFG_TDEFE      0x01            // event detection for new temperature data
#define REGPRESS_CTRL1          0x26            // 1byte control register 1
#define REGPRESS_CTRL3          0x28            // 1byte control register 3 - interrupt pin config
#define REGPRESS_CTRL4          0x29            // 1byte control register 4 - interrupt enable register
#define REGPRESS_CTRL5          0x2A            // 1byte control register 5 - interrupt cfg. register
#define PREG_CTRL3_IPOL1        0x20            // SET: INT1 pin active high
#define PREG_CTRL3_PPOD1        0x10            // SET: open drain output
#define PREG_CTRL3_IPOL2        0x02            // SET: INT2 pin active high
#define PREG_CTRL3_PPOD2        0x01            // SET: open drain output

#define PREG_CTRL4_DRDY         0x80            // SET: enable data ready interrupt

#define PREG_CTRL5_DRDY         0x80            // SET: data ready interrupt routed to INT1, RESET: routed to INT2 pin

#define PREG_CTRL1_ALT          0x80            // SET: altimeter mode, RESET: barometer mode
#define PREG_CTRL1_RAW          0x40            // SET: raw data output mode - data directly from sensor - The FIFO must be disabled and all other functionality: Alarms, Deltas, and other interrupts are disabled
#define PREG_CTRL1_OSMASK       0x38            // 3bit oversample ratio - it is 2^x,  0 - means 1 sample, 7 means 128 sample, see enum EPressOversampleRatio
#define PREG_CTRL1_RST          0x04            // SET: software reset
#define PREG_CTRL1_OST          0x02            // SET: initiate a measurement immediately. If the SBYB bit is set to active, setting the OST bit will initiate an immediate measurement, the part will then return to acquiring data as per the setting of the ST bits in CTRL_REG2. In this mode, the OST bit does not clear itself and must be cleared and set again to initiate another immediate measurement. One Shot: When SBYB is 0, the OST bit is an auto-clear bit. When OST is set, the device initiates a measurement by going into active mode. Once a Pressure/Altitude and Temperature measurement is completed, it clears the OST bit and comes back to STANDBY mode. User shall read the value of the OST bit before writing to this bit again
#define PREG_CTRL1_SBYB         0x01            // SET: sets the active mode. system makes periodic measurements set by ST in CTRL2 register.

#define REGPRESS_ID             (0x0C)            // 1byte pressure sensor chip ID


    const uint8     psens_set_01_data_event[] = { REGPRESS_DATACFG, (PREG_DATACFG_TDEFE | PREG_DATACFG_PDEFE | PREG_DATACFG_DREM) };     // set up data event signalling for pressure update
    const uint8     psens_set_02_interrupt_src[]  = { REGPRESS_CTRL3, PREG_CTRL3_IPOL1 };       // pushpull active high on INT1
    const uint8     psens_set_03_interrupt_en[]  = { REGPRESS_CTRL4, PREG_CTRL4_DRDY };         // enable data ready interrupt
    const uint8     psens_set_04_interrupt_out[]  = { REGPRESS_CTRL5, PREG_CTRL5_DRDY };        // route data ready interrupt to INT1

    const uint8     psens_cmd_sshot_baro[] = { REGPRESS_CTRL1, ( pos_4 | PREG_CTRL1_OST) };     // start one shot data aq. with 4 sample oversampling (~18ms wait time)

    if (i2c_barosensorcheck == 0)
    {
        return;
    }

    switch (i2c_barosensorcheck)
    {
        case 1:             /* init pressure sensor */
            while (HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER) != I2C_IDLE) {}
            HALI2C_Write(HALI2C_CHANNEL_BAROMETER, psens_set_01_data_event, sizeof(psens_set_01_data_event));
            while (HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER) != I2C_IDLE) {}
            HALI2C_Write(HALI2C_CHANNEL_BAROMETER, psens_set_02_interrupt_src, sizeof(psens_set_02_interrupt_src));
            while (HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER) != I2C_IDLE) {}
            HALI2C_Write(HALI2C_CHANNEL_BAROMETER, psens_set_03_interrupt_en, sizeof(psens_set_03_interrupt_en));
            while (HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER) != I2C_IDLE) {}
            HALI2C_Write(HALI2C_CHANNEL_BAROMETER, psens_set_04_interrupt_out, sizeof(psens_set_04_interrupt_out));
            break;
        case 2:             /* read ID */
            while (HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER) != I2C_IDLE) {}
            HALI2C_ReadRegister(HALI2C_CHANNEL_BAROMETER, REGPRESS_ID,  buff_1, 1);
            while (HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER) != I2C_IDLE) {}
            break;
        case 3:             /* read pressure */
            while (HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER) != I2C_IDLE) {}
            HALI2C_Write(HALI2C_CHANNEL_BAROMETER, psens_cmd_sshot_baro, sizeof(psens_cmd_sshot_baro));
            while (HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER) != I2C_IDLE) {}
            /* wait here with debugger - int. pin should be asserted by the sensor */
            HALI2C_ReadRegister(HALI2C_CHANNEL_BAROMETER, REGPRESS_OUTP,  buff_1, 5);
            while (HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER) != I2C_IDLE) {}
            break;
    }

    i2c_barosensorcheck = 0;
}



static void local_i2c_accel_sensor_check(void)
{
    /*
     *  FS[1:0]=10 -> 8g range, resolution 1/256 G     12bit resolution
     */
    #define REG1_CFG_VAL         ((2 << 3))     // Data rate 200Hz

    if (i2c_accelsensorcheck == 0)
    {
        return;
    }

    switch (i2c_accelsensorcheck)
    {
        case 1:             /* check / init */

            /* sensor check */
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}
            HALI2C_ReadRegister(HALI2C_CHANNEL_ACCELERO, 0x0D, buff_1, 1);      // 0x2A - WHOAMI
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}
            if (buff_1[0] == 0x2A)
            {
                asm("nop");     // result is good
            }
            /* initialization */
            buff_1[0] = 0x0E;       // XYZ_DATA_CFG register
            buff_1[1] = 0x02;       // set 8 G range
            HALI2C_Write(HALI2C_CHANNEL_ACCELERO, buff_1, 2);
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}

            buff_1[0] = 0x2A;       // CTRL_REG1
            buff_1[1] = REG1_CFG_VAL;
            HALI2C_Write(HALI2C_CHANNEL_ACCELERO, buff_1, 2);
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}

            //CTRL_REG2 - remains 0x00

            buff_1[0] = 0x2C;       // CTRL_REG3
            buff_1[1] = 0x02;       // - interrupt polarity: high,  - use push-pull
            HALI2C_Write(HALI2C_CHANNEL_ACCELERO, buff_1, 2);
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}

            buff_1[0] = 0x2D;       // CTRL_REG4
            buff_1[1] = 0x01;       // data ready int. enabled
            HALI2C_Write(HALI2C_CHANNEL_ACCELERO, buff_1, 2);
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}

            buff_1[0] = 0x2E;       // CTRL_REG5
            buff_1[1] = 0x01;       // data ready int. on pin INT1
            HALI2C_Write(HALI2C_CHANNEL_ACCELERO, buff_1, 2);
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}

            break;
        case 2:
            /* initialization - bulk */
            buff_1[0] = 0x0E;       // XYZ_DATA_CFG register
            buff_1[1] = 0x02;       // set 8 G range
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}
            HALI2C_Write(HALI2C_CHANNEL_ACCELERO, buff_1, 2);
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}

            buff_1[0] = 0x2A;       // CTRL_REG1
            buff_1[1] = REG1_CFG_VAL;
            buff_1[2] = 0x00;
            buff_1[3] = 0x02;       // - interrupt polarity: high,  - use push-pull
            buff_1[4] = 0x01;       // data ready int. enabled
            buff_1[5] = 0x01;       // data ready int. on pin INT1
            HALI2C_Write(HALI2C_CHANNEL_ACCELERO, buff_1, 6);
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}
            break;
        case 3:             /* read data set */
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}
            HALI2C_ReadRegister(HALI2C_CHANNEL_ACCELERO, 0x01, buff_1, 6);      // 0x2A - WHOAMI
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}
            break;

        case 4:             /* cyclic read */
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}
            while (i2c_accelsensorcheck)
            {
                if (PORT_GENPIN_ACC_INT())
                {
                    HALI2C_ReadRegister(HALI2C_CHANNEL_ACCELERO, 0x01, buff_1, 6);      // 0x2A - WHOAMI
                    while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}
                }
            }

            break;

        case 5:             /* set active mode */
            buff_1[0] = 0x2A;       // CTRL_REG1
            buff_1[1] = REG1_CFG_VAL | 0x01;
            HALI2C_Write(HALI2C_CHANNEL_ACCELERO, buff_1, 2);
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}
            break;
        case 6:             /* set stdby mode */
            buff_1[0] = 0x2A;       // CTRL_REG1
            buff_1[1] = REG1_CFG_VAL | 0x00;
            HALI2C_Write(HALI2C_CHANNEL_ACCELERO, buff_1, 2);
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}
            break;
        case 7:             /* set selftest */
            buff_1[0] = 0x2B;       // CTRL_REG1
            buff_1[1] = 0x80;
            HALI2C_Write(HALI2C_CHANNEL_ACCELERO, buff_1, 2);
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}
            break;
        case 8:             /* clear selftest */
            buff_1[0] = 0x2B;       // CTRL_REG1
            buff_1[1] = 0x00;
            HALI2C_Write(HALI2C_CHANNEL_ACCELERO, buff_1, 2);
            while (HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO) != I2C_IDLE) {}
            break;
    }

    i2c_accelsensorcheck = 0;
}





#endif
