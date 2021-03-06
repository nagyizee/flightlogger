#ifndef	_RTAPPPACK_H
#define _RTAPPPACK_H
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
    RTAPPPACK_ST_INIT = 0,        /* Initialization */
    RTAPPPACK_ST_IDLE,
    RTAPPPACK_ST_WRITEPAGE,
    RTAPPPACK_ST_WRITENVM,
    RTAPPPACK_ST_ERROR
} tRtAppPackInternalStates;

/*--------------------------------------------------
 *                              Defines
 *--------------------------------------------------*/
#define RTAPPPACK_PAGESIZE      256         /* Flash Page size 256 bytes */
     
/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/

void RtAppPack_Init(void);
void RtAppPack_Main(uint8 timebase);

#endif

