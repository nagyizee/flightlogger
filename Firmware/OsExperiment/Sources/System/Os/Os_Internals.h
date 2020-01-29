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


/*--------------------------------------------------
 * 		    		 Types
 *--------------------------------------------------*/

typedef struct
{
	uint32 tstamp_current;


} tOsInternalInstance;

/*--------------------------------------------------
 *   		  	 	Functions
 *--------------------------------------------------*/

/* init the system and other runtime stuff - configurable per project */
void OsRt_Init(void);


#endif


