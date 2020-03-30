#ifndef	_RTAPPDATA_H
#define _RTAPPDATA_H
/**
 *
 *
 *
 */

/*--------------------------------------------------
 *                              Includes
 *--------------------------------------------------*/
#include "base.h"

/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/
typedef enum
{
    RTAPPDATA_ST_INIT = 0,        /* Initialization */
    RTAPPDATA_ST_IDLE,
    RTAPPDATA_ST_WRITEPAGE,
    RTAPPDATA_ST_WRITENVM,
    RTAPPDATA_ST_ERROR
} tRtAppDataInternalStates;

/*--------------------------------------------------
 *                              Defines
 *--------------------------------------------------*/
#define RTAPPDATA_PAGESIZE      256         /* Flash Page size 256 bytes */
#define RTAPPDATA_STARTADDRESS  0x00004000  /* Start address of data area */
#define RTAPPDATA_ENDADDRESS    0x00010000  /* End address of data area */
     
/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/

void RtAppData_Init(void);     
void RtAppData_Main(void);

tResult RtAppData_SavePage(uint8* buffer);

#endif

