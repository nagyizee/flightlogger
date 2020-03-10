/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

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


static void local_PollInternalStateMachine(bool timetick);


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
    lI2c.ch_status = I2C_IDLE;
}

void HALI2C_MainFunction(void)
{
    local_PolInternalStateMachine(true);
}

tI2CStatus HALI2C_GetStatus(tI2CChannelType ch)
{
    local_PollInternalStateMachine(false);
    return lI2c.ch_status;
}

tResult HALI2C_Write(tI2CChannelType ch, const uint8 *buffer, uint32 size)
{
    if (lI2c.ch_status == I2C_BUSY)
    {
        return RES_BUSY;
    }
    /* clean up the events */
    NRF_TWIM0->EVENTS_STOPPED = 0;
    NRF_TWIM0->EVENTS_ERROR = 0;
    /* set up device address, buffer to transmit and data amount */
    NRF_TWIM0->ADDRESS = lI2cConfig[ch].dev_addr;
    NRF_TWIM0->TXD.PTR = (uint32)buffer;
    NRF_TWIM0->TXD.MAXCNT = size;
    /* link the last.tx event to the stop task */
    NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTTX_STOP_Msk;
    /* start the transfer */
    NRF_TWIM0->TASKS_STARTTX = 1;
    lI2c.ch_status = I2C_BUSY;

    return RES_OK;
}

tResult HALI2C_Read(tI2CChannelType ch, uint8 *buffer, uint32 size)
{
    return RES_OK;
}

tResult HALI2C_ReadRegister(tI2CChannelType ch, uint8 dev_register, uint8 *buffer, uint32 size)
{
    return RES_OK;
}


/*--------------------------------------------------
 *             Local functions
 *--------------------------------------------------*/

static void local_PollInternalStateMachine(bool timetick)
{
    if (lI2c.ch_status == I2C_BUSY)
    {
        if (NRF_TWIM0->EVENTS_STOPPED)
        {
            /* operation successful -> go to idle */
            lI2c.ch_status = I2C_IDLE;
        }
        /* check the errors also - will override the idle */
        if (NRF_TWIM0->EVENTS_ERROR)
        {
            /* In this peripheral only the NAK errors are detected - no arbitration check */
            lI2c.ch_status = I2C_ERR_NAK;
            /* give stop condition and wait for it's completion */
            NRF_TWIM0->TASKS_STOP = 1;
            while (NRF_TWIM0->EVENTS_STOPPED == 0);
            /* clear the error source */
            NRF_TWIM0->ERRORSRC = TWIM_ERRORSRC_DNACK_Msk | TWIM_ERRORSRC_ANACK_Msk;
        }
    }
}

