/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include <string.h>
#include "base.h"
#include "NxpBaro.h"
#include "NxpBaro_Internals.h"
#include "HALI2c.h"
#include "HALPort.h"

/*--------------------------------------------------
 *                  Defines
 *--------------------------------------------------*/


/*--------------------------------------------------
 *         Local Variables and prototypes
 *--------------------------------------------------*/

#define local_GetSensorIntrPin()                PORT_GETPIN_PRESS_INT()

#define LENGTH_INI_SEQ      (4)

const tBaroDrvInitStruct lIniSeq[LENGTH_INI_SEQ] = {
        { REGPRESS_DATACFG, (PREG_DATACFG_TDEFE | PREG_DATACFG_PDEFE | PREG_DATACFG_DREM) }, /* set up data event signaling for pressure update */
        { REGPRESS_CTRL3, PREG_CTRL3_IPOL1 },       /* pushpull active high on INT1 */
        { REGPRESS_CTRL4, PREG_CTRL4_DRDY },        /* enable data ready interrupt */
        { REGPRESS_CTRL5, PREG_CTRL5_DRDY }         /* route data ready interrupt to INT1 */
};

/* start one shot data aq. with 16 sample oversampling (~66ms wait time) */
const uint8     psens_cmd_sshot_baro[]  = { REGPRESS_CTRL1, ( pos_16 | PREG_CTRL1_OST) };
/* start one shot data aq. with 16 sample oversampling (~66ms wait time) */
const uint8     psens_cmd_sshot_alti[]  = { REGPRESS_CTRL1, ( pos_16 | PREG_CTRL1_OST | PREG_CTRL1_ALT) };




static tBaroDrvInternals    lBaro;


static void local_StateRead(void);
static void local_StateInit(void);

static void local_SetStateIdle(void);

static void local_ReadSetupReadOp(void);
static void local_ReadProcessResult(void);
static void local_ReadHandleFailure(tResult result);

static uint32 local_ConvertTempFromRaw(uint8 *rawbuff);

/*--------------------------------------------------
 *             Interface Functions
 *--------------------------------------------------*/


void NXPBaro_Init(void)
{
    memset(&lBaro, 0, sizeof(tBaroDrvInternals));
    lBaro.state = drvst_initializing;
    lBaro.substate = BARODRV_INISTATE_CHECKDEVICE;
    lBaro.fail_ctr = NXPBARO_CFG_ERROR_RETRY;
}

void NXPBaro_MainFunction(void)
{
    if (lBaro.state == drvst_idle)
    {
        return;     /* do it quick */
    }
    switch (lBaro.state)
    {
        case drvst_sensor_read:
            local_StateRead();
            break;
        case drvst_initializing:
            local_StateInit();
            break;
        case drvst_error:
            /* just do nothing */
            break;
    }
}

tNxpBaroStatus NXPBaro_GetStatus(uint32 *acq_mask)
{
    if (acq_mask)
    {
        *acq_mask = (uint32)lBaro.meas_compl_mask;
    }

    switch (lBaro.state)
    {
        case drvst_idle:
            if (lBaro.meas_compl_mask)
            {
                return NXPBARO_ST_READY;
            }
            else
            {
                return NXPBARO_ST_IDLE;
            }
        case drvst_sensor_read:
            return NXPBARO_ST_BUSY;
        case drvst_error:
            return NXPBARO_ST_ERROR;
    }
    return NXPBARO_ST_UNINITTED;
}

void NXPBaro_Sleep(void)
{
    /* not implemented - since no fifo/cyclic read is supported yet, we use one shot mode - device is in low power if no acquision is required */
}

tResult NXPBaro_Acquire(uint32 mask)
{
    /* exit the function if something is not right */
    if ((lBaro.state == drvst_sensor_read) ||
        (lBaro.state == drvst_initializing))
    {
        return RES_BUSY;
    }
    else if (lBaro.state == drvst_error)
    {
        return RES_ERROR;
    }

    mask &= NXPBARO_ACQMASK_PRESSURE | NXPBARO_ACQMASK_ALTITUDE | NXPBARO_ACQMASK_TEMP;
    if (mask == 0)
    {
        return RES_OK;
    }

    /* start the acquisition state machine */
    lBaro.meas_compl_mask = 0;
    lBaro.meas_req_mask = (uint8)mask;
    lBaro.state = drvst_sensor_read;
    lBaro.substate = drvsst_read_oneshot;
    /* not to lose much time - do the first poll */
    local_StateRead();
    return RES_OK;
}

uint32 NXPBaro_GetResult(tNxpBaroMeasurementSelector select)
{
    uint32 selected;
    uint32 retval;

    retval = NXPBARO_ERROR_CODE;

    selected = select & (uint32)lBaro.meas_compl_mask;
    if (selected & NXPBARO_ACQMASK_PRESSURE)
    {
        retval = lBaro.mval_baro;
        lBaro.meas_compl_mask &= ~NXPBARO_ACQMASK_PRESSURE;
    }
    else if (selected & NXPBARO_ACQMASK_ALTITUDE)
    {
        retval = lBaro.mval_alti;
        lBaro.meas_compl_mask &= ~NXPBARO_ACQMASK_ALTITUDE;
    }
    else if(selected & NXPBARO_ACQMASK_TEMP)
    {
        retval = lBaro.mval_temp;
        lBaro.meas_compl_mask &= ~NXPBARO_ACQMASK_TEMP;
    }

    return retval;
}


/*--------------------------------------------------
 *             Local Functions
 *--------------------------------------------------*/

static void local_StateRead(void)
{
    tI2CStatus i2c_res;
    uint32  op_mask;

    /* precheck for I2C availability and operation result */
    i2c_res = HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER);
    if (i2c_res != I2C_IDLE)
    {
        if ((i2c_res == I2C_BUSY_CH) ||
            (i2c_res == I2C_BUSY_OTHER))
        {
            /* exit unconditionally for:    _oneshot, _waitevent    - these need free i2c peripheral
             *    only for channel busy:    _waitresult             - no further op. on i2c, just wait for RX finish for baro. channel */
            if ((lBaro.substate != drvsst_read_waitresult) || (i2c_res == I2C_BUSY_CH))
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
    switch (lBaro.substate)
    {
        case drvsst_read_waitevent:
            if (local_GetSensorIntrPin())
            {
                local_ReadSetupReadOp();
                lBaro.substate = drvsst_read_waitresult;
            }
            else
            {
                lBaro.timeout_ctr--;
                if (lBaro.timeout_ctr == 0)
                {
                    local_ReadHandleFailure(RES_TIMEOUT);
                }
            }
            break;
        case drvsst_read_oneshot:
            /* launch one shot read command */
            op_mask = ((uint32)lBaro.meas_compl_mask ^ (uint32)lBaro.meas_req_mask) & ((uint32)lBaro.meas_req_mask);
            if (op_mask & NXPBARO_ACQMASK_PRESSURE)  /* pressure has priority over altitude */
            {
                /* read barometric pressure */
                /* NOTE - Nordic can not transfer DMA from Flash - so no const can be used */
                lBaro.hw_buff[0] = psens_cmd_sshot_baro[0];
                lBaro.hw_buff[1] = psens_cmd_sshot_baro[1];
            }
            else
            {
                /* read altitude */
                lBaro.hw_buff[0] = psens_cmd_sshot_alti[0];
                lBaro.hw_buff[1] = psens_cmd_sshot_alti[1];
            }
            HALI2C_Write(HALI2C_CHANNEL_BAROMETER, lBaro.hw_buff, 2);

            lBaro.timeout_ctr = NXPBARO_CFG_TIMEROUT_CTR;
            lBaro.substate = drvsst_read_waitevent;
            break;
        case drvsst_read_waitresult:
            local_ReadProcessResult();
            op_mask = ((uint32)lBaro.meas_compl_mask ^ (uint32)lBaro.meas_req_mask) & ((uint32)lBaro.meas_req_mask);
            if (op_mask)
            {
                /* if we have an alitmerty measurement - do it as a new oneshot */
                lBaro.meas_compl_mask = (uint8)op_mask;
                lBaro.substate = drvsst_read_oneshot;
            }
            else
            {
                /* we are finished */
                local_SetStateIdle();
            }
            break;
    }
}

static void local_StateInit(void)
{
    tI2CStatus i2c_res;
    tResult failure;

    i2c_res = HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER);
    failure = RES_OK;
    switch (i2c_res)
    {
        case I2C_BUSY_CH:
        case I2C_BUSY_OTHER:
            /* i2c busy - nothing to do - exit */
            return;
        case I2C_IDLE:
            if (lBaro.substate == BARODRV_INISTATE_CHECKDEVICE)
            {
                /* trigger the device ID readback for first */
                HALI2C_ReadRegister(HALI2C_CHANNEL_BAROMETER, REGPRESS_ID, lBaro.hw_buff, 1);
                lBaro.substate = 0;     /* begin the instruction list */
            }
            else if (lBaro.substate < LENGTH_INI_SEQ)
            {
                /* before the first init. instruction - check the prew. read device ID */
                if ((lBaro.substate == 0) && (lBaro.hw_buff[0] != PREG_ID_VALUE))
                {
                    failure = RES_INVALID;
                    break;
                }
                /* NOTE - Nordic can not transfer DMA from Flash - so no const can be used */
                lBaro.hw_buff[0] = lIniSeq[lBaro.substate].reg[0];
                lBaro.hw_buff[1] = lIniSeq[lBaro.substate].reg[1];
                HALI2C_Write(HALI2C_CHANNEL_BAROMETER, lBaro.hw_buff, 2);
                lBaro.substate++;
            }
            else
            {
                /* reached the end of the command list - go to idle state */
                local_SetStateIdle();
            }
            return;
        default:
            failure = RES_ERROR;
            return;
    }

    /* handle the errors  */
    if (failure != RES_OK)
    {
        lBaro.fail_ctr--;
        if (lBaro.fail_ctr == 0)
        {
            /* all error retrials failed - enter in error state */
            lBaro.state = drvst_error;
            lBaro.substate = 0;
            lBaro.error_code = failure;
        }
        else
        {
            /* restart init */
            lBaro.substate = BARODRV_INISTATE_CHECKDEVICE;
        }
    }
}


static void local_SetStateIdle(void)
{
    lBaro.state = drvst_idle;
    lBaro.substate = 0;
    lBaro.fail_ctr = NXPBARO_CFG_ERROR_RETRY;
}


static void local_ReadSetupReadOp(void)
{
    /* all I2C operations are quaranteed to be successfull by the caller - no error check is required */
    uint32  op_mask;

    op_mask = ((uint32)lBaro.meas_compl_mask ^ (uint32)lBaro.meas_req_mask) & ((uint32)lBaro.meas_req_mask);

    if (op_mask == NXPBARO_ACQMASK_TEMP)
    {
        /* read temperature only */
        HALI2C_ReadRegister(HALI2C_CHANNEL_BAROMETER, REGPRESS_OUTT, lBaro.hw_buff, 2);
    }
    else if ((op_mask & NXPBARO_ACQMASK_TEMP) == 0)
    {
        /* read pressure/altitude only */
        HALI2C_ReadRegister(HALI2C_CHANNEL_BAROMETER, REGPRESS_OUTP, lBaro.hw_buff, 3);  /* read temperature also */
    }
    else
    {
        /* read both */
        HALI2C_ReadRegister(HALI2C_CHANNEL_BAROMETER, REGPRESS_OUTP, lBaro.hw_buff, 5);  /* read temperature also */
    }
}


static void local_ReadProcessResult(void)
{
    /* result is received in the lBaro.hw_read_val buffer */
    uint32  op_mask;
    int32   tempreg;
    op_mask = ((uint32)lBaro.meas_compl_mask ^ (uint32)lBaro.meas_req_mask) & ((uint32)lBaro.meas_req_mask);

    if (op_mask == NXPBARO_ACQMASK_TEMP)
    {
        /* read temperature only */
        lBaro.mval_temp = local_ConvertTempFromRaw(&lBaro.hw_buff[0]);
        lBaro.meas_compl_mask |= NXPBARO_ACQMASK_TEMP;
    }
    else
    {
        if (op_mask & NXPBARO_ACQMASK_TEMP)
        {
            /* read both */
            lBaro.mval_temp = local_ConvertTempFromRaw(&lBaro.hw_buff[3]);
            lBaro.meas_compl_mask |= NXPBARO_ACQMASK_TEMP;
        }

        if (lBaro.meas_req_mask & NXPBARO_ACQMASK_PRESSURE)         /* pressure has priority over altitude */
        {
            lBaro.mval_baro = ((((uint32)lBaro.hw_buff[0] << 24) |
                                ((uint32)lBaro.hw_buff[1] << 16) |
                                ((uint32)lBaro.hw_buff[2] << 8)) >> 6);
            lBaro.meas_compl_mask |= NXPBARO_ACQMASK_PRESSURE;
        }
        else
        {
            tempreg = (int32)((int16)(((uint16)lBaro.hw_buff[0] << 8) | (uint16)lBaro.hw_buff[1]));
            tempreg *= (1 << 16);
            tempreg += (NXPBARO_ALT_OFFSET << 16);
            lBaro.mval_alti = (uint32)tempreg + ((uint32)lBaro.hw_buff[2] << 8);
            lBaro.meas_compl_mask |= NXPBARO_ACQMASK_ALTITUDE;
        }
    }

}


static void local_ReadHandleFailure(tResult result)
{
    lBaro.fail_ctr--;
    if (lBaro.fail_ctr == 0)
    {
        /* all error retrials failed - enter in error state */
        lBaro.state = drvst_error;
        lBaro.substate = 0;
        lBaro.error_code = result;
    }
    else
    {
        /* restart init */
        lBaro.substate = drvsst_read_oneshot;
    }
}


static uint32 local_ConvertTempFromRaw(uint8 *rawbuff)
{
    uint32 retval;

    retval = (uint32)(((int32)((int8)rawbuff[0]) * (1 << 16)) + (NXPBARO_TEMP_OFFSET * ( 1 << 8)));
    retval += ((uint32)rawbuff[1] << 8);

    return retval;
}
