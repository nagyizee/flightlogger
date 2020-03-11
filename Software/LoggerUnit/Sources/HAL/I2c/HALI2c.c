/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include <string.h>
#include "base.h"
#include "HALI2c.h"
#include "HALI2c_Internals.h"
#include "HALport.h"
#include "nrf.h"

/*--------------------------------------------------
 *                  Defines
 *--------------------------------------------------*/




/*--------------------------------------------------
 *         Local Variables and prototypes
 *--------------------------------------------------*/

    /* primitive hardcoded implementation */
static tI2cInternals       lI2c;
static tI2cChannelConfig   lI2cConfig[] =
{
    { HALI2C_CHADDR_BAROMETER },
    { HALI2C_CHADDR_ACCELERO },
};

static uint8 lRegVal;

static void local_PollInternalStateMachine(bool timetick);
static void local_I2cOpPrepare(tI2CChannelType ch);

/*--------------------------------------------------
 *             Interface Functions
 *--------------------------------------------------*/

void HALI2C_Init(void)
{
    /* primitive hardcoded implementation */

    /* enable peripheral and set up baudrate, output pins, etc. */
    NRF_TWIM0->ENABLE = TWIM_ENABLE_ENABLE_Enabled;
    NRF_TWIM0->PSEL.SCL = PIN_I2C_SCK;
    NRF_TWIM0->PSEL.SDA = PIN_I2C_SD;
    NRF_TWIM0->FREQUENCY = I2C_BAUDRATE_250;
    /* clear error flags if any */
    NRF_TWIM0->ERRORSRC = TWIM_ERRORSRC_DNACK_Msk | TWIM_ERRORSRC_ANACK_Msk;

    /* initialize local variables */
    memset(&lI2c, 0, sizeof(tI2cInternals));
}

void HALI2C_MainFunction(void)
{
    local_PolInternalStateMachine(true);
}

tI2CStatus HALI2C_GetStatus(tI2CChannelType ch)
{
    local_PollInternalStateMachine(false);
    if (lI2c.busy)
    {
        return I2C_BUSY;
    }
    else
    {
        return lI2c.ch_status[ch];
    }
}

tResult HALI2C_Write(tI2CChannelType ch, const uint8 *buffer, uint32 size)
{
    if (lI2c.busy)
    {
        return RES_BUSY;
    }

    local_I2cOpPrepare(ch);

    /* set up device address, buffer to transmit and data amount */
    NRF_TWIM0->ADDRESS = lI2cConfig[ch].dev_addr;
    NRF_TWIM0->TXD.PTR = (uint32)buffer;
    NRF_TWIM0->TXD.MAXCNT = size;
    /* link the last.tx event to the stop task */
    NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTTX_STOP_Msk;
    /* start the transfer */
    NRF_TWIM0->TASKS_STARTTX = 1;
    return RES_OK;
}

tResult HALI2C_Read(tI2CChannelType ch, uint8 *buffer, uint32 size)
{
    if (lI2c.busy)
    {
        return RES_BUSY;
    }

    local_I2cOpPrepare(ch);

    /* set up device address, buffer to transmit and data amount */
    NRF_TWIM0->ADDRESS = lI2cConfig[ch].dev_addr;
    NRF_TWIM0->RXD.PTR = (uint32)buffer;
    NRF_TWIM0->RXD.MAXCNT = size;
    /* link the last.tx event to the stop task */
    NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTRX_STOP_Msk;
    /* start the transfer */
    NRF_TWIM0->TASKS_STARTRX = 1;
    return RES_OK;
}

tResult HALI2C_ReadRegister(tI2CChannelType ch, uint8 dev_register, uint8 *buffer, uint32 size)
{
    if (lI2c.busy)
    {
        return RES_BUSY;
    }

    local_I2cOpPrepare(ch);

    lRegVal = dev_register;
    /* set up device address, buffer to transmit and data amount */
    NRF_TWIM0->ADDRESS = lI2cConfig[ch].dev_addr;
    NRF_TWIM0->TXD.PTR = (uint32)(&lRegVal);
    NRF_TWIM0->TXD.MAXCNT = 1;
    NRF_TWIM0->RXD.PTR = (uint32)buffer;
    NRF_TWIM0->RXD.MAXCNT = size;
    /* link the last.tx event to the stop task */
    NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTTX_STARTRX_Msk | TWIM_SHORTS_LASTRX_STOP_Msk;
    /* start the transfer */
    NRF_TWIM0->TASKS_STARTTX = 1;
    return RES_OK;
}


/*--------------------------------------------------
 *             Local functions
 *--------------------------------------------------*/

static void local_PollInternalStateMachine(bool timetick)
{
    if (lI2c.busy)
    {
        if (NRF_TWIM0->EVENTS_STOPPED)
        {
            /* operation successful -> go to idle */
            lI2c.ch_status[lI2c.op_ch] = I2C_IDLE;
            lI2c.busy = false;
        }
        /* check the errors also - will override the idle */
        if (NRF_TWIM0->EVENTS_ERROR)
        {
            /* In this peripheral only the NAK errors are detected - no arbitration check */
            lI2c.ch_status[lI2c.op_ch] = I2C_ERR_NAK;
            lI2c.busy = false;
            /* give stop condition and wait for it's completion */
            NRF_TWIM0->TASKS_STOP = 1;
            while (NRF_TWIM0->EVENTS_STOPPED == 0);
            /* clear the error source */
            NRF_TWIM0->ERRORSRC = TWIM_ERRORSRC_DNACK_Msk | TWIM_ERRORSRC_ANACK_Msk;
        }
    }
}

static void local_I2cOpPrepare(tI2CChannelType ch)
{
    /* prepare channel status */
    lI2c.busy = true;
    lI2c.op_ch = ch;
    lI2c.ch_status[ch] = I2C_IDLE;      /* since new op. on this channel - delete the error status */

    /* clean up the events */
    NRF_TWIM0->EVENTS_STOPPED = 0;
    NRF_TWIM0->EVENTS_ERROR = 0;
}



