/* Cypress S25FL064 External flash driver - using standard SPI interface */

/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include "base.h"
#include "CypFlash.h"
#include "HALSpi.h"

/*--------------------------------------------------
 *                  Defines
 *--------------------------------------------------*/


/*--------------------------------------------------
 *         Local Variables and prototypes
 *--------------------------------------------------*/

static tCypFlashStatus          lCypFlash_Status = CYPFLASH_ST_UNINITIALIZED;
static tCypFlashDeviceStatus    lCypFlash_DeviceStatus;

/* Device handling functions */
static void local_CypFlash_GetDeviceStatus(void);

/*--------------------------------------------------
 *             Interface Functions
 *--------------------------------------------------*/


void CypFlash_Init(void)
{

}

void CypFlash_MainFunction(void)
{

}

tCypFlashStatus CypFlash_GetStatus(void)
{
  return lCypFlash_Status;
}

/* Local functions */
static void local_CypFlash_GetDeviceStatus(void)
{
  /* Get the HW status register from the device */
  
}