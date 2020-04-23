/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include <string.h>
#include "base.h"
#include "NxpAccel.h"
#include "NxpAccel_Internals.h"
#include "HALI2c.h"
#include "HALPort.h"

/*--------------------------------------------------
 *                  Defines
 *--------------------------------------------------*/

#define local_GetSensorIntrPin()                PORT_GENPIN_ACC_INT()

/*--------------------------------------------------
 *         Local Variables and prototypes
 *--------------------------------------------------*/

static tAccDrvInternals    lAcc;


static void local_StateInit(void);
static void local_StateRead(void);

static void local_StateReadProcessSelftest(void);
static void local_StateReadProcessResult(bool save_result);

static void local_ReadHandleFailure(tResult result);
static void local_SetStateIdle(void);

static void local_ProcessRawResults(uint8 *rawbuff, tNxpAccelResult *results);

/*--------------------------------------------------
 *             Interface Functions
 *--------------------------------------------------*/

void NXPAccel_Init(void)
{
    memset(&lAcc, 0, sizeof(tAccDrvInternals));
    lAcc.state = accdrvst_initializing;
    lAcc.substate = SUBST_INI_CHIPTEST;
    lAcc.fail_ctr = NXPACCEL_CFG_ERROR_RETRY;
}

void NXPAccel_MainFunction(void)
{
    if (lAcc.state == accdrvst_idle)
    {
        return;     /* do it quick */
    }
    switch (lAcc.state)
    {
        case accdrvst_sensor_read:
            local_StateRead();
            break;
        case accdrvst_initializing:
            local_StateInit();
            break;
        case accdrvst_error:
            /* just do nothing */
            break;
    }
}

tNxpAccelStatus NXPAccel_GetStatus()
{
    if (lAcc.state == accdrvst_error)
    {
        if (lAcc.error_code == RES_FAILED)
        {
            return NXPACCEL_ST_SENS_ERROR;
        }
        else
        {
            return NXPACCEL_ST_HW_ERROR;
        }
    }
    else if (lAcc.fifo_c)
    {
        return NXPACCEL_ST_READY;
    }
    else if (lAcc.state == accdrvst_sensor_read)
    {
        return NXPACCEL_ST_BUSY;
    }
    else if (lAcc.state == accdrvst_idle)
    {
        return NXPACCEL_ST_IDLE;
    }
    else
    {
        return NXPACCEL_ST_INITIALIZING;
    }
}

void NXPAccel_Sleep(void)
{
    /* deactivate continuous acquisition - when read is finished the sensor will enter in standby */
    if (lAcc.state == accdrvst_sensor_read)
    {
        lAcc.acq_cont = ACQ_CONT_SHTDN;
    }
    else
    {
        lAcc.acq_cont = ACQ_CONT_NONE;
    }
}

tResult NXPAccel_Acquire(uint32 acq_type)
{
    if (lAcc.state == accdrvst_error)
    {
        return RES_ERROR;
    }

    if (acq_type == NXPACCEL_ACQ_CONTINUOUS)
    {
        lAcc.acq_cont = ACQ_CONT_ON;
    }
    else
    {
        lAcc.acq_cont = ACQ_CONT_NONE;
        if (lAcc.state != accdrvst_idle)
        {
            return RES_BUSY;
        }
    }

    if (lAcc.state == accdrvst_idle)
    {
        /* initiate read */
        lAcc.state = accdrvst_sensor_read;
        lAcc.substate = SUBST_READ_NORMALMODE;
        local_StateRead();
    }
    
    return RES_OK;
}

tResult NXPAccel_GetResult(tNxpAccelResult *result)
{
    if (lAcc.state == accdrvst_error)
    {
        return RES_ERROR;
    }
    if ((lAcc.fifo_c == 0) || (result == NULL))
    {
        return RES_INVALID;
    }

    *result = lAcc.fifo_data[lAcc.fifo_r];
    lAcc.fifo_c--;
    lAcc.fifo_r++;
    if (lAcc.fifo_r == ACC_MAX_FIFO_ELEMS)
    {
        lAcc.fifo_r = 0;
    }
    return RES_OK;
}


/*--------------------------------------------------
 *             Local Functions
 *--------------------------------------------------*/

static void local_StateInit(void)
{
    tI2CStatus i2c_res;
    tResult failure;

    i2c_res = HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO);
    failure = RES_OK;
    switch (i2c_res)
    {
        case I2C_BUSY_CH:
        case I2C_BUSY_OTHER:
            /* i2c busy - nothing to do - exit */
            return;
        case I2C_IDLE:
            if (lAcc.substate == SUBST_INI_CHIPTEST)
            {
                /* trigger the device ID readback for first */
                HALI2C_ReadRegister(HALI2C_CHANNEL_ACCELERO, REGACC_ID, lAcc.hw_buff, 1);
                lAcc.substate = SUBST_INI_INISET1;          /* begin the instruction list */
            }
            else if (lAcc.substate == SUBST_INI_INISET1)
            {
                /* before the first init. instruction - check the prew. read device ID */
                if (lAcc.hw_buff[0] != AREG_ID_VALUE)
                {
                    failure = RES_INVALID;
                    break;
                }
                /* init the dynamic range */
                lAcc.hw_buff[0] = REGACC_XYZDATACFG;
                lAcc.hw_buff[1] = AREG_XYZDATACFG_FS8;
                HALI2C_Write(HALI2C_CHANNEL_ACCELERO, lAcc.hw_buff, 2);
                lAcc.substate = SUBST_INI_INISET2;
            }
            else if (lAcc.substate == SUBST_INI_INISET2)
            {
                /* do the bulk initialization */
                lAcc.hw_buff[0] = REGACC_CTRL1;
                lAcc.hw_buff[1] = AREG_CFG1_DR_200 | AREG_CFG1_ACTIVE;           // CTRL_REG1: data rate at 200Hz
                lAcc.hw_buff[2] = AREG_CFG2_SELFTEST | AREG_CFG2_MOD_NORMAL;     // CTRL_REG2: selftest activated, use normal power scheme
                lAcc.hw_buff[3] = AREG_CFG3_IPOL_H;                              // CTRL_REG3: interrupt polarity: high,  - use push-pull
                lAcc.hw_buff[4] = AREG_CFG4_INTEN_DRDY;                          // CTRL_REG4: data ready int. enabled
                lAcc.hw_buff[5] = AREG_CFG5_INTPIN_DRDY;                         // CTRL_REG5: data ready int. on pin INT1
                HALI2C_Write(HALI2C_CHANNEL_ACCELERO, lAcc.hw_buff, 6);
                lAcc.substate = SUBST_INI_SELFTEST;
            }
            else if (lAcc.substate == SUBST_INI_SELFTEST)
            {
                /* read the result of the selftest */
                if (PORT_GENPIN_ACC_INT())
                {
                    HALI2C_ReadRegister(HALI2C_CHANNEL_ACCELERO, REGACC_OUTX, lAcc.hw_buff, 6);
                    lAcc.substate = SUBST_INI_SELFTEST_RESULT;
                }
            }
            else if (lAcc.substate == SUBST_INI_SELFTEST_RESULT)
            {
                /* result is in from I2C - save it */
                local_ProcessRawResults(lAcc.hw_buff, &lAcc.selftst_data);
                /* disable selftest and leave it running with normal mode */
                lAcc.hw_buff[0] = REGACC_CTRL2;
                lAcc.hw_buff[1] = AREG_CFG2_MOD_NORMAL;                          // CTRL_REG2: selftest deactivated, use normal power scheme
                HALI2C_Write(HALI2C_CHANNEL_ACCELERO, lAcc.hw_buff, 2);
                lAcc.substate = SUBST_INI_FINALIZE;
            }
            else
            {
                /* end of initialization - go in the read mode to read the non-offsetted result for selftest */
                lAcc.state = accdrvst_sensor_read;
                lAcc.substate = SUBST_READ_WAITEVENT;
                lAcc.timeout_ctr = NXPBARO_CFG_TIMEROUT_CTR;
                /* reset fail counter for the selftest (only if first start-up) */
                if (lAcc.selftest == 0)
                {
                    lAcc.fail_ctr = NXPACCEL_CFG_ERROR_RETRY;
                }
                /* do two measurement loops in normal mode */
                lAcc.selftest = 2;
            }
            return;
        default:
            failure = RES_ERROR;
            return;
    }

    /* handle the errors  */
    if (failure != RES_OK)
    {
        lAcc.fail_ctr--;
        if (lAcc.fail_ctr == 0)
        {
            /* all error retrials failed - enter in error state */
            lAcc.state = accdrvst_error;
            lAcc.substate = 0;
            lAcc.error_code = failure;
        }
        else
        {
            /* restart init */
            lAcc.substate = SUBST_INI_CHIPTEST;
        }
    }
}

static void local_StateRead(void)
{
    tI2CStatus i2c_res;

    /* precheck for I2C availability and operation result */
    i2c_res = HALI2C_GetStatus(HALI2C_CHANNEL_ACCELERO);
    if (i2c_res != I2C_IDLE)
    {
        if ((i2c_res == I2C_BUSY_CH) ||
            (i2c_res == I2C_BUSY_OTHER))
        {
            /* exit unconditionally for:    _oneshot, _waitevent, _waitresult (single)  - these need free i2c peripheral
             *    only for channel busy:    _waitresult (multi)                         - no further op. on i2c, just wait for RX finish for baro. channel */
            if ((lAcc.substate != SUBST_READ_WAITRESULT) ||
                (i2c_res == I2C_BUSY_CH) ||
                (lAcc.acq_cont != ACQ_CONT_ON))                     /* in single mode we need to deactivate active mode - there we need I2C write access */
            {
                return;
            }
        }
        else
        {
            local_ReadHandleFailure(RES_ERROR);
            return;
        }
    }

    /* run read state machine */
    switch (lAcc.substate)
    {
        case SUBST_READ_WAITEVENT:
            if (local_GetSensorIntrPin())
            {
                HALI2C_ReadRegister(HALI2C_CHANNEL_ACCELERO, REGACC_OUTX, lAcc.hw_buff, 6);
                lAcc.substate = SUBST_READ_WAITRESULT;
            }
            else
            {
                lAcc.timeout_ctr--;
                if (lAcc.timeout_ctr == 0)
                {
                    local_ReadHandleFailure(RES_TIMEOUT);
                }
            }
            break;
        case SUBST_READ_NORMALMODE:
            /* launch one shot read command */
            lAcc.hw_buff[0] = REGACC_CTRL1;
            lAcc.hw_buff[1] = AREG_CFG1_DR_200 | AREG_CFG1_ACTIVE;
            HALI2C_Write(HALI2C_CHANNEL_ACCELERO, lAcc.hw_buff, 2);

            lAcc.timeout_ctr = NXPBARO_CFG_TIMEROUT_CTR;
            lAcc.substate = SUBST_READ_WAITEVENT;
            break;
        case SUBST_READ_WAITRESULT:
            /* process the received result */
            if (lAcc.selftest)
            {
                local_StateReadProcessSelftest();
            }
            else
            {
                local_StateReadProcessResult(true);
            }
            break;
        case SUBST_READ_SHUTDOWN:
            if (local_GetSensorIntrPin())
            {
                /* in case if one more result is available in sensor - discard it */
                HALI2C_ReadRegister(HALI2C_CHANNEL_ACCELERO, REGACC_OUTX, lAcc.hw_buff, 6);
            }
            if (lAcc.acq_cont == ACQ_CONT_SHTDN)
            {
                /* if continuous mode is shut down or read stopped - clean up the fifo */
                lAcc.fifo_r = 0;
                lAcc.fifo_c = 0;
                lAcc.fifo_w = 0;
                lAcc.acq_cont = ACQ_CONT_NONE;
            }
            local_SetStateIdle();
            break;
    }
}


static void local_StateReadProcessSelftest(void)
{
    tNxpAccelResult result;
    int32 diff;

    lAcc.selftest--;
    if (lAcc.selftest == 0)
    {
        /* did the second run - check the value */
        local_ProcessRawResults(lAcc.hw_buff, &result);

        diff = (int32)lAcc.selftst_data.Z - (int32)result.Z;
        if ((diff > SELFTEST_DIFF_MIN) && (diff < SELFTEST_DIFF_MAX))
        {
            /* result is good - run the normal read state machine - but without saving the current result */
            local_StateReadProcessResult(false);
        }
        else
        {
            if (lAcc.fail_ctr)
            {
                /* failed result - run the selftest again */
                lAcc.fail_ctr--;
                /* marking that it is not the first run - do not reset the retrial counter */
                lAcc.selftest = 2;
                /* jump to the init step 2 - the one with the selftest setup */
                lAcc.state = accdrvst_initializing;
                lAcc.substate = SUBST_INI_INISET2;
            }
            else
            {
                /* failure counter is up - mark as failed sensor */
                lAcc.state = accdrvst_error;
                lAcc.substate = 0;
                lAcc.error_code = RES_FAILED;
            }
        }
    }
    else
    {
        /* one more read */
        lAcc.substate = SUBST_READ_WAITEVENT;
        lAcc.timeout_ctr = NXPBARO_CFG_TIMEROUT_CTR;
    }
}

static void local_StateReadProcessResult(bool save_result)
{
    tNxpAccelResult result;

    if (save_result)
    {
        local_ProcessRawResults(lAcc.hw_buff, &result);
        /* store result only if fifo has free space */
        if (lAcc.fifo_c < ACC_MAX_FIFO_ELEMS)
        {
            lAcc.fifo_data[lAcc.fifo_w] = result;
            lAcc.fifo_c++;
            lAcc.fifo_w++;
            if (lAcc.fifo_w < ACC_MAX_FIFO_ELEMS)
            {
                lAcc.fifo_w = 0;
            }
        }
    }

    if (lAcc.acq_cont == ACQ_CONT_ON)
    {
        /* in case of continuous acquisition go back to wait for event state - remain in read state */
        lAcc.substate = SUBST_READ_WAITRESULT;
        lAcc.timeout_ctr = NXPBARO_CFG_TIMEROUT_CTR;
    }
    else
    {
        /* in single acquisition we need to stop the sensor */
        lAcc.hw_buff[0] = REGACC_CTRL1;
        lAcc.hw_buff[1] = AREG_CFG1_DR_200;
        HALI2C_Write(HALI2C_CHANNEL_ACCELERO, lAcc.hw_buff, 2);

        lAcc.substate = SUBST_READ_SHUTDOWN;
    }
}

static void local_ReadHandleFailure(tResult result)
{
    lAcc.fail_ctr--;
    if (lAcc.fail_ctr == 0)
    {
        /* all error retrials failed - enter in error state */
        lAcc.state = accdrvst_error;
        lAcc.substate = 0;
        lAcc.error_code = result;
    }
    else
    {
        lAcc.substate = SUBST_READ_NORMALMODE;
    }
}

static void local_SetStateIdle(void)
{
    lAcc.state = accdrvst_idle;
    lAcc.substate = 0;
    lAcc.fail_ctr = NXPBARO_CFG_ERROR_RETRY;
}


static void local_ProcessRawResults(uint8 *rawbuff, tNxpAccelResult *results)
{
    int16 *outres;
    uint16 value;
    uint32 i;

    /* make it "for" cycle usable */
    outres = &(results->X);

    /* convert the raw data to 12bit signed values in 16bit signed containers */
    for (i = 0; i < 3; i++)
    {
        value = ((uint16)rawbuff[i * 2] << 8) | rawbuff[i * 2 + 1];
        outres[i] = ((int16)value) / 16;
    }
}


