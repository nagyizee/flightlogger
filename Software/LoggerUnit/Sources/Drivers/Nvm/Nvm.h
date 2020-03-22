#ifndef	_NVM_H
#define _NVM_H
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
    NVM_BLOCKSTATE_DEFAULT     = 0,
    NVM_BLOCKSTATE_HISTORY,
    NVM_BLOCKSTATE_NVM,
    NVM_BLOCKSTATE_DIRTY,
    NVM_BLOCKSTATE_WR_MAIN,     /* Written to the main sector, writing to mirror still ongoing */
    NVM_MEMORY_FULL,
    NVM_DATA_UNCHANGED,
    NVM_PARAM_ERROR
}tNvmBlockStatus;

typedef uint8 tNvmStatus;

/*--------------------------------------------------
 *                              Defines
 *--------------------------------------------------*/
/* tNvmStatus bit mapping definitions */
#define NVM_MEM_ST_BLOCKFOUND   0x01
#define NVM_MEM_ST_STORAGE_FULL 0x80

/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/

void Nvm_Init(void);     
void Nvm_Main(void);

tNvmStatus Nvm_GetStatus(void);
void Nvm_PurgeHistory(void);

tNvmBlockStatus Nvm_ReadBlock(uint8 blockID, uint8* buffer, uint16 count);
tNvmBlockStatus Nvm_WriteBlock(uint8 blockID, uint8* buffer, uint16 count);

#endif
