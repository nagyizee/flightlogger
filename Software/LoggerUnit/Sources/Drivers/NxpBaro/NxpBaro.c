/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include "base.h"
#include "NxpBaro.h"
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


void NXPBaro_Init(void)
{

}

void NXPBaro_MainFunction(void)
{

}

tNxpBaroStatus NXPBaro_GetStatus(uint32 *acq_mask)
{
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
