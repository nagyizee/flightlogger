#include "LibCrc.h"

uint8 CalcCRC8(uint8 startval, uint8* buffer, uint16 counter)
{
    uint8 acc = startval;
    uint16 i;
    
    for (i=0; i<counter; i++) acc += (buffer[i]+i)*buffer[i];
    return acc;
}