#ifndef	_EXAMPLERTAPP_H
#define _EXAMPLERTAPP_H
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

#define EXAMPLEAPPACTIVE  /* Comment to deactivate application test routines */

/*--------------------------------------------------
 *   		  	 	Functions
 *--------------------------------------------------*/

#ifdef EXAMPLEAPPACTIVE
/* Init the cpu core items (clocks and stuff) */
void ExampleRtApp_Main(uint32 taskIdx);
#endif

#endif

