#ifndef	_BASE_H
#define _BASE_H
/**
 *		Base type defines through the project
 */

/*--------------------------------------------------
 * 		    		Includes
 *--------------------------------------------------*/

#include <stdint.h>

/*--------------------------------------------------
 * 		    		Defines
 *--------------------------------------------------*/

#ifndef NULL
#define NULL    0
#endif

#define RES_OK		    (0)
#define RES_BUSY        (1)
#define RES_INVALID     (2)
#define RES_ERROR       (3)
#define RES_TIMEOUT     (4)

#ifndef ON_QT_PLATFORM

#define true    1
#define false   0

typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint64_t uint64;
typedef int64_t int64;
typedef uint8_t bool;

/*returned error code: see RES_XXX defines above */
typedef uint32 tResult;

#else

#define true    1
#define false   0

typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned long long uint64;
typedef long long int64;
#ifndef __cplusplus
typedef unsigned char bool;
#endif

typedef uint32 tResult;

#endif

#endif

