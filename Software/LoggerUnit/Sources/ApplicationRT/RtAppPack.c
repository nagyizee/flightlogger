/* Logging data packer module */

/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include <string.h>
#include "base.h"
#include "RtAppPack.h"
#include "RtAppMeasurement.h"
#include "RtAppData.h"

/*--------------------------------------------------
 *                  Defines and type definitions
 *--------------------------------------------------*/

typedef struct
{
    uint8                       Buffer[RTAPPPACK_PAGESIZE];
    tRtAppPackInternalStates    InternalState;
}tRtAppPackStruct;

/*--------------------------------------------------
 *                  Local Variables and prototypes
 *--------------------------------------------------*/

static tRtAppPackStruct  lRtAppPack;

/*--------------------------------------------------
 *                  Interface Functions
 *--------------------------------------------------*/

void RtAppPack_Init(void)
{
    /* initialize local variables */
    memset(&lRtAppPack, 0, sizeof(tRtAppPackStruct));
}

void RtAppPack_Main(uint8 timebase)
{
    switch(lRtAppPack.InternalState)
    {   
    case RTAPPPACK_ST_IDLE:
        {
            break;
        }
    case RTAPPPACK_ST_INIT:
        {
            break;
        }
    case RTAPPPACK_ST_WRITEPAGE:
        {
            break;
        }
    case RTAPPPACK_ST_WRITENVM:
        {
            break;
        }
    case RTAPPPACK_ST_ERROR:
        {
            break;
        }
    default:
        {
            lRtAppPack.InternalState = RTAPPPACK_ST_ERROR;
        }
    }
}

/*--------------------------------------------------
 *             Local functions
 *--------------------------------------------------*/


