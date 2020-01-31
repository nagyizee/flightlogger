#ifndef	_OS_INTERNALS_H
#define _OS_INTERNALS_H
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
 * 		    		Defines
 *--------------------------------------------------*/

#define OSINT_DEAD_TIME			(10)		/* nr. of Os timer cycles considered for dead time which is needed for OS housekeeping after each task call */

/*--------------------------------------------------
 * 		    		 Types
 *--------------------------------------------------*/

typedef struct
{
	uint32 task_idx;
	uint32 tstamp_current;


} tOsInternalInstance;

/*--------------------------------------------------
 *   		  	 	Functions
 *--------------------------------------------------*/

/* init the system and other runtime stuff - configurable per project */
void OsRt_Init(void);


#endif


