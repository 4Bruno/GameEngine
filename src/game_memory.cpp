#include "game.h"

memory_arena *
ThreadBeginArena(thread_memory_arena * ThreadArena)
{
    Assert(ThreadArena->InUse);
    memory_arena * Arena = &ThreadArena->Arena;
    Assert(Arena->Base && Arena->MaxSize > 0);
    Arena->CurrentSize = 0;
    return Arena;
}
void
ThreadEndArena(thread_memory_arena * ThreadArena)
{
    Assert(ThreadArena->InUse);
    ThreadArena->Arena.CurrentSize = 0;
    ThreadArena->InUse = false;
}

inline uint8 *
_PushSize(memory_arena * Arena,uint32 Size)
{
    Assert((Arena->CurrentSize + Size) < Arena->MaxSize);
    uint8 * BaseAddr = Arena->Base + Arena->CurrentSize;
    Arena->CurrentSize += Size;
    return BaseAddr;
}

void
InitializeArena(memory_arena * Arena,uint8 * BaseAddr, uint32 MaxSize)
{
    Arena->MaxSize = MaxSize;
    Arena->CurrentSize = 0;
    Arena->Base = BaseAddr;
}

/*
 *  This is single thread proc
 *  Fetching thread arenas should be done in the main thread
 *  and no available arena case must be handle properly
 *  Any critical step that must be guaranteed to be handle
 *  in the current frame must be done using CompleteQueue 
 */
thread_memory_arena *
GetThreadArena(game_state * GameState)
{
    thread_memory_arena * ThreadArena = 0;

    for (uint32 ThreadArenaIndex = 0;
                ThreadArenaIndex < GameState->LimitThreadArenas;
                ++ThreadArenaIndex)
    {
        thread_memory_arena * TestThreadArena = GameState->ThreadArena + ThreadArenaIndex;
        if (!TestThreadArena->InUse)
        {
            TestThreadArena->InUse = true;
            ThreadArena = TestThreadArena;
            break;
        }
    }

    return ThreadArena;
}
