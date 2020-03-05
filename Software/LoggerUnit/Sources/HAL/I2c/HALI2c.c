/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include "base.h"
#include "HALI2c.h"

/*--------------------------------------------------
 *                  Defines
 *--------------------------------------------------*/


/*--------------------------------------------------
 *         Local Variables and prototypes
 *--------------------------------------------------*/


/*--------------------------------------------------
 *             Interface Functions
 *--------------------------------------------------*/

void HALI2C_Init(void)
{

}

void HALI2C_MainFunction(void)
{

}

tI2CStatus HALI2C_GetStatus(tI2CChannelType ch)
{

    return I2C_IDLE;
}

tResult HALI2C_Write(tI2CChannelType ch, const uint8 *buffer, uint32 size)
{
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
