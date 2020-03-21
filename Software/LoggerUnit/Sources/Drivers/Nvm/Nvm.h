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
    NVM_PARAM_ERROR
}tNvmBlockState;

/*--------------------------------------------------
 *                              Defines
 *--------------------------------------------------*/

/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/

void Nvm_Init(void);     
void Nvm_Main(void);

void Nvm_PurgeHistory(void);

tNvmBlockState Nvm_ReadBlock(uint8 blockID, uint8* buffer, uint16 count);
tNvmBlockState Nvm_WriteBlock(uint8 blockID, uint8* buffer, uint16 count);

#endif
