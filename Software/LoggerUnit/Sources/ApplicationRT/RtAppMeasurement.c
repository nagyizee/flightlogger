/* Measurement module */

/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include <string.h>
#include "base.h"
#include "RtAppMeasurement.h"
#include "RtAppSensor.h"

/*--------------------------------------------------
 *                  Defines and type definitions
 *--------------------------------------------------*/

typedef struct
{
    tRtAppMeasInternalStates    InternalState;
}tRtAppMeasStruct;

/*--------------------------------------------------
 *                  Local Variables and prototypes
 *--------------------------------------------------*/

static tRtAppMeasStruct  lRtAppMeas;

/*--------------------------------------------------
 *                  Interface Functions
 *--------------------------------------------------*/

void RtAppMeas_Init(void)
{
    /* initialize local variables */
    memset(&lRtAppMeas, 0, sizeof(tRtAppMeasStruct));
}

void RtAppMeas_Main(void)
{
    switch(lRtAppMeas.InternalState)
    {   
    case RTAPPMEAS_ST_IDLE:
        {
            break;
        }
    case RTAPPMEAS_ST_INIT:
        {
            break;
        }
    case RTAPPMEAS_ST_ERROR:
        {
            break;
        }
    default:
        {
            lRtAppMeas.InternalState = RTAPPMEAS_ST_ERROR;
        }
    }
}

/*--------------------------------------------------
 *             Local functions
 *--------------------------------------------------*/


