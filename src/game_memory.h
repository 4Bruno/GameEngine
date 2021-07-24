#ifndef GAME_MEMORY_H

#include "game_platform.h"

struct memory_arena
{
    uint8 * Base;
    uint32 MaxSize;
    uint32 CurrentSize;
};

#define PushSize(Arena,Size) _PushSize(Arena,Size*sizeof(char))
#define PushArray(Arena,Count,Struct) (Struct *)_PushSize(Arena,Count*sizeof(Struct))
#define PushStruct(Arena,Struct) (Struct *)PushArray(Arena,1,Struct)

inline uint8 *
_PushSize(memory_arena * Arena,uint32 Size)
{
    Assert((Arena->CurrentSize + Size) < Arena->MaxSize);
    uint8 * BaseAddr = Arena->Base + Arena->CurrentSize;
    Arena->CurrentSize += Size;
    return BaseAddr;
}




#define GAME_MEMORY_H
#endif
