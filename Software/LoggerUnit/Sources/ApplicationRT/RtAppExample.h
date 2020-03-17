#ifndef	_RTAPPEXAMPLE_H
#define _RTAPPEXAMPLE_H
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

#define RTAPPEXAMPLEACTIVE  /* Comment to deactivate application test routines */

/*--------------------------------------------------
 *   		  	 	Functions
 *--------------------------------------------------*/

#ifdef RTAPPEXAMPLEACTIVE
/* Init the cpu core items (clocks and stuff) */
void RtAppExample_Main(uint32 taskIdx);
#endif

#endif

