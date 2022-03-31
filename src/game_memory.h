#ifndef GAME_MEMORY_H
#include "game_platform.h"

struct memory_arena
{
    u8 * Base;
    u32 MaxSize;
    u32 CurrentSize;

    // Arena might or not be used as temp memory
    // this will be used by begin/end temparena
    u32 StackTemporaryMemory;
};

struct thread_memory_arena
{
    b32 InUse;
    memory_arena Arena;
};

struct memory_aligned_result
{
    uintptr_t AddressAligned;
    u32 Delta;
};


#define PushSize(Arena,Size) _PushSize(Arena,Size*sizeof(char))
#define PushArray(Arena,Count,Struct) (Struct *)_PushSize(Arena,Count*sizeof(Struct))
#define PushStruct(Arena,Struct) (Struct *)PushArray(Arena,1,Struct)

#define BeginTempArena(Arena,ID) u32 Arena##SizeBegin##ID = (Arena)->CurrentSize;\
                                 ++Arena->StackTemporaryMemory;
#define EndTempArena(Arena,ID) --Arena->StackTemporaryMemory;\
                               Arena->CurrentSize = Arena##SizeBegin##ID;
#define TempArenaSanityCheck(Arena) Assert(Arena->StackTemporaryMemory == 0)

GAME_API inline u8 *
_PushSize(memory_arena * Arena,u32 Size)
{
    Assert((Arena->CurrentSize + Size) <= Arena->MaxSize);
    u8 * BaseAddr = Arena->Base + Arena->CurrentSize;
    Arena->CurrentSize += Size;
    return BaseAddr;
}

GAME_API memory_arena *
ThreadBeginArena(thread_memory_arena * ThreadArena);

GAME_API void
ThreadEndArena(thread_memory_arena * ThreadArena);

GAME_API void
InitializeArena(memory_arena * Arena,u8 * BaseAddr, u32 MaxSize);

GAME_API void
SubArena(memory_arena * ParentArena,memory_arena * Arena, u32 Size);

/*
 *  ONLY MAIN THREAD CAN USE THIS METHOD
 *  This is single thread proc
 *  Fetching thread arenas should be done in the main thread
 *  and no available arena case must be handle properly
 *  Any critical step that must be guaranteed to be handle
 *  in the current frame must be done using CompleteQueue 
 */
GAME_API thread_memory_arena *
GetThreadArena(game_state * GameState, u32 MinDesiredSize = 0);

GAME_API thread_memory_arena *
GetThreadArena(thread_memory_arena * ThreadArenaArray, u32 LimitThreadArenas, u32 MinDesiredSize = 0);

GAME_API void
MemCopy(u8 * Dest,u8 * Src,u32 EntitySize);

GAME_API void
Memset(u8 * Dest,u8 c,u32 Size);

GAME_API memory_aligned_result
AlignMemoryAddress(void * Addr,u32 Align);

#define GAME_MEMORY_H
#endif
