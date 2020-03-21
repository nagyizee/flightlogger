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

/*--------------------------------------------------
                                Type definitions
 *--------------------------------------------------*/

typedef enum
{
    NVM_BLOCKSTATE_DEFAULT     = 0,
    NVM_BLOCKSTATE_HISTORY,
    NVM_BLOCKSTATE_NVM,
    NVM_BLOCKSTATE_DIRTY,
    NVM_BLOCKSTATE_WR_MAIN /* Written to the main sector, writing to mirror still ongoing */
}tNvmBlockState;

/*--------------------------------------------------
 *                              Defines
 *--------------------------------------------------*/

/*--------------------------------------------------
 *                              Exported interfaces
 *--------------------------------------------------*/

void Nvm_Init(void);     
void Nvm_Main(void);

#endif
