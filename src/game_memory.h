#ifndef GAME_MEMORY_H
#include "game_platform.h"

struct memory_arena
{
    u8 * Base;
    u32 MaxSize;
    u32 CurrentSize;
};

struct thread_memory_arena
{
    b32 InUse;
    memory_arena Arena;
};


#define PushSize(Arena,Size) _PushSize(Arena,Size*sizeof(char))
#define PushArray(Arena,Count,Struct) (Struct *)_PushSize(Arena,Count*sizeof(Struct))
#define PushStruct(Arena,Struct) (Struct *)PushArray(Arena,1,Struct)

GAME_API inline u8 *
_PushSize(memory_arena * Arena,u32 Size);

GAME_API memory_arena *
ThreadBeginArena(thread_memory_arena * ThreadArena);

GAME_API void
ThreadEndArena(thread_memory_arena * ThreadArena);

GAME_API void
InitializeArena(memory_arena * Arena,u8 * BaseAddr, u32 MaxSize);

/*
 *  ONLY MAIN THREAD CAN USE THIS METHOD
 *  This is single thread proc
 *  Fetching thread arenas should be done in the main thread
 *  and no available arena case must be handle properly
 *  Any critical step that must be guaranteed to be handle
 *  in the current frame must be done using CompleteQueue 
 */
GAME_API thread_memory_arena *
GetThreadArena(game_state * GameState);

GAME_API void
MemCopy(u8 * Dest,u8 * Src,u32 EntitySize);

#define GAME_MEMORY_H
#endif
