/* NVM and Storage Memory Manager */

/*--------------------------------------------------
 *                  Includes
 *--------------------------------------------------*/

#include <string.h>
#include "Nvm_Cfg.h"

#ifdef NVM_USE_CYPFLASH
#include "CypFlash.h"
#endif

/*--------------------------------------------------
 *                  Exported variables
 *--------------------------------------------------*/
#ifdef NVM_USE_RAM
    #ifdef NVM_USE_CYPFLASH
        #error /* Only 1 data storage should be active at any time !!! */
    #endif
    
/* emulated in RAM */
     uint8 lNvmRamBuffer[NVM_DATASET_CNT*NVM_SECTOR_CNT*NVM_SECTOR_SIZE]; 
#endif

/*--------------------------------------------------
 *                  Exported config data
 *--------------------------------------------------*/

const tNvmBlockStruct cNvmBlockConfig[NVM_NUMBER_OF_BLOCKS] = 
{   /*  BlockAddress    BlockSize   MagicWord*/
    {   {0x0000},       { 8},       {0x1D}},       /* Block 0 - Device identification */
    {   {0x0008},       { 8},       {0xEE}},       /* Block 1 - Device Health status */
    {   {0x0010},       {16},       {0xCE}}        /* Block 2 - Comm/Flight Event */
};

const uint8 cNvmDefaultData[NVM_TOTAL_SIZE_OF_BLOCKS] =
{   /* Block 0 - Device identification - size 8 bytes */
    /* Magic    HW Ver  SW Version  Unique ID   UID Inverted */
        0x1D,   0x01,   0x00,0x01,  0xFF,0xFF,   0xFF,0xFF, 

    /* Block 1 - Device Health status - size 8 bytes */
    /* Magic        DateTimestamp     Error code + data    */
        0xEE,   0xFF,0xFF,0xFF,0xFF,    0xFF,   0xFF,0xFF,
        
    /* Block 2 Comm Event */
    /* Magic    Date        Time        JudgeID     EventCode    */
        0xCE,   0xFF,0xFF,  0xFF,0xFF,  0xFF,0xFF,    0xFF,
            /*  USERid      AirplaneID  CategoryID      reserved */
                0xFF,0xFF,  0xFF,0xFF,  0xFF,       0xFF,0xFF,0xFF
};

/*--------------------------------------------------
 *                  Local Support Functions
 *--------------------------------------------------*/

#ifdef NVM_USE_CYPFLASH
static tResult i_NvmTranslateStatus(tCypFlashStatus status);
#endif

/*--------------------------------------------------
 *                  Interface Functions
 *--------------------------------------------------*/

tResult Nvm_InitInternal(void)
{
    tResult retval = RES_OK;
    /* Check memory source availability */
#ifdef NVM_USE_RAM
    memset(&lNvmRamBuffer, NVM_ERASE_VAL, NVM_DATASET_CNT*NVM_SECTOR_CNT*NVM_SECTOR_SIZE );
#endif
    
#ifdef NVM_USE_CYPFLASH
    if (CypFlash_GetStatus() != CYPFLASH_ST_READY)
    {
        retval = RES_BUSY;
    }
#endif
    return retval;
}

void Nvm_MemoryMain(uint8 tick)
{
#ifdef NVM_USE_CYPFLASH
    CypFlash_Main(tick);
#endif
}

tResult Nvm_GetMemoryStatus(void)
{
#ifdef NVM_USE_RAM
    return RES_OK;
#endif
    
#ifdef NVM_USE_CYPFLASH
    tCypFlashStatus fls_status = CypFlash_GetStatus();
    return i_NvmTranslateStatus(fls_status);
#endif
}

tResult Nvm_StartEraseSector(uint32 address)
{
#ifdef NVM_USE_RAM
    memset(lNvmRamBuffer+address, NVM_ERASE_VAL, NVM_SECTOR_SIZE);
    return RES_OK;
#endif
    
#ifdef NVM_USE_CYPFLASH
    tCypFlashStatus fls_status = CypFlash_EraseSector(address);
    return i_NvmTranslateStatus(fls_status);    
#endif
}

tResult Nvm_ReadMemory(uint32 address, uint16 count, uint8* buffer)
{
#ifdef NVM_USE_RAM
    memcpy(buffer, lNvmRamBuffer+address, count);
    return RES_OK;
#endif
    
#ifdef NVM_USE_CYPFLASH
    tCypFlashStatus fls_status = CypFlash_Read(address, count, buffer);
    return i_NvmTranslateStatus(fls_status);
#endif
}

tResult Nvm_WriteMemory(uint32 address, uint16 count, uint8* buffer)
{
#ifdef NVM_USE_RAM
    memcpy(lNvmRamBuffer+address, buffer, count);
    return RES_OK;
#endif
    
#ifdef NVM_USE_CYPFLASH
    tCypFlashStatus fls_status = CypFlash_Write(address, count, buffer);
    return i_NvmTranslateStatus(fls_status);    
#endif
}

#ifdef NVM_USE_CYPFLASH
static tResult i_NvmTranslateStatus(tCypFlashStatus status)
{
    tResult retval;
    if (status == CYPFLASH_ST_READY)
    {
        retval = RES_OK;
    }
    else if (status == CYPFLASH_ST_BUSY)
    {
        retval = RES_BUSY;
    }
    else if (status < CYPFLASH_ST_READY)
    {
        retval = RES_INVALID;
    }
    else
    retval = RES_ERROR;
    
    return retval;
}
#endif