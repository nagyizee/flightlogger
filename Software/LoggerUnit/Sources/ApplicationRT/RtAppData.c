/* Logging data manager */

/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include <string.h>
#include "base.h"
#include "RtAppData.h"
#include "CypFlash.h"

/*--------------------------------------------------
 *                  Defines and type definitions
 *--------------------------------------------------*/

typedef struct
{
    uint8                       Buffer[RTAPPDATA_PAGESIZE];
    tRtAppDataInternalStates    InternalState;
}tRtAppDataStruct;

/*--------------------------------------------------
 *                  Local Variables and prototypes
 *--------------------------------------------------*/

static tRtAppDataStruct  lRtAppData;

/*--------------------------------------------------
 *                  Interface Functions
 *--------------------------------------------------*/

void RtAppData_Init(void)
{
    /* initialize local variables */
    memset(&lRtAppData, 0, sizeof(tRtAppDataStruct));

    while(lRtAppData.InternalState < RTAPPDATA_ST_IDLE)
    {
        RtAppData_Main();
        CypFlash_Main(2); /* 100 calls timeout */
    }
}

void RtAppData_Main(void)
{
    switch(lRtAppData.InternalState)
    {   
    case RTAPPDATA_ST_IDLE:
        {
            break;
        }
    case RTAPPDATA_ST_INIT:
        {
            break;
        }
    case RTAPPDATA_ST_WRITEPAGE:
        {
            break;
        }
    case RTAPPDATA_ST_WRITENVM:
        {
            break;
        }
    case RTAPPDATA_ST_ERROR:
        {
            break;
        }
    default:
        {
            lRtAppData.InternalState = RTAPPDATA_ST_ERROR;
        }
    }
}

/*--------------------------------------------------
 *             Local functions
 *--------------------------------------------------*/
