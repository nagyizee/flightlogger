/* CRC calculation library */

/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/
#include "LibCrc.h"
#include "LibCrc_Cfg.h"
/*--------------------------------------------------
 *                  Defines and type definitions
 *--------------------------------------------------*/

/*--------------------------------------------------
 *                  Local Variables and prototypes
 *--------------------------------------------------*/

#ifdef CRC8_USE_TABLES
static uint8    crc_tab8_init = 0;
static uint8    crc_tab8[256];

static void init_crc8_tab( void );
#endif

/*--------------------------------------------------
 *                  Interface Functions
 *--------------------------------------------------*/

uint8 CalcCRC8(uint8 startval, uint8* buffer, uint16 length)
{
    uint8   crc = startval; /* calculation variable */
    uint16  i;              /* byte counter */

#ifdef CRC8_USE_TABLES
    
    /* Init table inside the first function call */
    if ( ! crc_tab8_init ) init_crc8_tab();
    
    for (i=0; i<length; i++)
    {
        crc = crc_tab8[(buffer[i]^crc)];
    }

#else    
    uint8   bit;            /* bit mask */
    
    for (i=0; i<length; i++) 
    {
        crc ^= buffer[i];
        for (bit = 8; bit>0; --bit)
        {
            if (crc & 0x80) 
                crc = (crc << 1) ^ CRC8_POLYNOMIAL;
            else
                crc = crc << 1;
        }
    }
#endif    
    return crc;
}


/*--------------------------------------------------
 *             Local functions
 *--------------------------------------------------*/

#ifdef CRC8_USE_TABLES

static void init_crc8_tab( void )
{
    uint16  i;
	uint16  j;
	uint8   crc;
	uint16  c;

	for (i=0; i<256; i++) {

		crc = 0;
		c   = i;

		for (j=8; j>0; --j) 
        {

			if ( (crc ^ c) & 0x80 ) crc = ( crc << 1 ) ^ CRC8_POLYNOMIAL;
			else                    crc =   crc << 1;

			c = c << 1;
		}
		crc_tab8[i] = crc;
	}

	crc_tab8_init = 1;
}

#endif