#ifndef	_RTAPPMEAS_H
#define _RTAPPMEAS_H
/**
 *
 *
 *
 */

/*--------------------------------------------------
 * 		    		Includes
 *--------------------------------------------------*/

#include "base.h"

/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/
typedef enum
{
    RTAPPMEAS_ST_INIT = 0,        /* Initialization */
    RTAPPMEAS_ST_IDLE,
    RTAPPMEAS_ST_ERROR
} tRtAppMeasInternalStates;

/*--------------------------------------------------
 *                              Defines
 *--------------------------------------------------*/
     
/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/

void RtAppMeas_Init(void);
void RtAppMeas_Main(void);

#endif

