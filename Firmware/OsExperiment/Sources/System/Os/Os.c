/*--------------------------------------------------
 * 		    		Includes
 *--------------------------------------------------*/

#include <string.h>
#include "Os.h"
#include "Os_Internals.h"
#include "Os_Rt.h"
#include "HALOsSys.h"

/*--------------------------------------------------
 * 		    		Defines
 *--------------------------------------------------*/

#define Os_Disable_Interrupts()		do { HALOsSys_DisableAllInterrupts(); } while (0)
#define Os_Enable_Interrupts()		do { HALOsSys_EnableAllInterrupts(); } while (0)

/*--------------------------------------------------
 * 	       Local Variables and prototypes
 *--------------------------------------------------*/

static tOsInternalInstance	lOs;


static void local_InitInternals(void);

/*--------------------------------------------------
 *   		   Interface Functions
 *--------------------------------------------------*/

void Os_Init(void)
{
	Os_Disable_Interrupts();
	/* init runtime elements */
	OsRt_Init();
	/* init Os internals */
	local_InitInternals();

	/* init os hardware resources */
	HALOsSys_Init();
}

void Os_Run(void)
{
	while (1)
	{
		gOsRt_BackgndTask.taskItem();
	}
}


/*--------------------------------------------------
 *   		   Exported Function
 *--------------------------------------------------*/

void Os_ShedulerTimedTaskEntry(void)
{



}


/*--------------------------------------------------
 *   		   Local Functions
 *--------------------------------------------------*/

static void local_InitInternals(void)
{
	memset(&lOs, 0, sizeof(lOs));

}

