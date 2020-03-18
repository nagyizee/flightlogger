/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include <string.h>
#include "base.h"
#include "NxpBaro.h"
#include "NxpBaro_Internals.h"
#include "HALI2c.h"

/*--------------------------------------------------
 *                  Defines
 *--------------------------------------------------*/


/*--------------------------------------------------
 *         Local Variables and prototypes
 *--------------------------------------------------*/

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
static void local_StateLowPower(void);

/*--------------------------------------------------
 *             Interface Functions
 *--------------------------------------------------*/


void NXPBaro_Init(void)
{
    memset(&lBaro, 0, sizeof(tBaroDrvInternals));
}

void NXPBaro_MainFunction(void)
{
    if ((lBaro.state == drvst_idle) || (lBaro.state == drvst_low_power))
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
        case drvst_goto_low_power:
            local_StateLowPower();
            break;
    }
}

tNxpBaroStatus NXPBaro_GetStatus(uint32 *acq_mask)
{
    switch (lBaro.state)
    {
        case drvst_idle:
            if (lBaro.error_code)
            {
                return NXPBARO_ST_ERROR;
            }
            else
            {
                return NXPBARO_ST_IDLE;
            }
        case drvst_sensor_read:
            return NXPBARO_ST_BUSY;
        case drvst_low_power:
        case drvst_goto_low_power:
            return NXPBARO_ST_LOW_PWR;
    }
    return NXPBARO_ST_UNINITTED;
}

void NXPBaro_Sleep(void)
{

}

tResult NXPBaro_Acquire(uint32 mask)
{
    return RES_OK;
}

uint32 NXPBaro_GetResult(tNxpBaroMeasurementSelector select)
{
    return NXPBARO_ERROR_CODE;
}


/*--------------------------------------------------
 *             Local Functions
 *--------------------------------------------------*/

static void local_StateRead(void)
{

}

static void local_StateInit(void)
{
    tI2CStatus i2c_res;

    i2c_res = HALI2C_GetStatus(HALI2C_CHANNEL_BAROMETER);
    switch (i2c_res)
    {
        case I2C_BUSY:
            /* nothing to do - exit */
            return;
        case I2C_IDLE:
            if (lBaro.substate < LENGTH_INI_SEQ)
            {
                HALI2C_Write(HALI2C_CHANNEL_BAROMETER, &lIniSeq[lBaro.substate].reg[0], 2);\
                lBaro.substate++;
            }
            else
            {
                /* reached the end of the command list - go to idle state */
                lBaro.substate = 0;
                lBaro.state = drvst_idle;
            }
            return;
        default:

            return;
    }
}

static void local_StateLowPower(void)
{

}

