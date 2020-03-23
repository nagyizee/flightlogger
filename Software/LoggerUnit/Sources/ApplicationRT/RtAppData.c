/* Logging data manager */

/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include <string.h>
#include "base.h"
#include "RtAppData.h"

/*--------------------------------------------------
 *                  Defines and type definitions
 *--------------------------------------------------*/

typedef struct
{
    uint32 var;
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

//    while(lRtAppData.InternalState < NVDATA_ST_IDLE) RtAppData_Main();
}

void RtAppData_Main(void)
{
/*      switch(lRtAppData.InternalState)
    {   
    case NVDATA_ST_IDLE:
        {
            break;
        }
    }
*/
}

/*--------------------------------------------------
 *             Local functions
 *--------------------------------------------------*/
