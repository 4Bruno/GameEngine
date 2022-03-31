#include "game.h"
//TODO: remove this
#include <memory.h>

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

void
InitializeArena(memory_arena * Arena,u8 * BaseAddr, u32 MaxSize)
{
    Arena->MaxSize = MaxSize;
    Arena->CurrentSize = 0;
    Arena->Base = BaseAddr;
}

void
SubArena(memory_arena * ParentArena,memory_arena * Arena, u32 Size)
{
    Arena->Base = PushSize(ParentArena,Size);
    Arena->MaxSize = Size;
    Arena->CurrentSize = 0;
}

/*
 *  This is single thread proc
 *  Fetching thread arenas should be done in the main thread
 *  and no available arena case must be handle properly
 *  Any critical step that must be guaranteed to be handle
 *  in the current frame must be done using CompleteQueue 
 */
thread_memory_arena *
GetThreadArena(game_state * GameState, u32 MinDesiredSize)
{
    thread_memory_arena * ThreadArena = 0;

    for (u32 ThreadArenaIndex = 0;
                ThreadArenaIndex < GameState->LimitThreadArenas;
                ++ThreadArenaIndex)
    {
        thread_memory_arena * TestThreadArena = GameState->ThreadArena + ThreadArenaIndex;
        if (
                !TestThreadArena->InUse && 
                (MinDesiredSize == 0 || TestThreadArena->Arena.MaxSize >= MinDesiredSize)
            )
        {
            TestThreadArena->InUse = true;
            ThreadArena = TestThreadArena;
            break;
        }
    }

    return ThreadArena;
}

thread_memory_arena *
GetThreadArena(thread_memory_arena * ThreadArenaArray, u32 LimitThreadArenas, u32 MinDesiredSize)
{
    thread_memory_arena * ThreadArena = 0;

    for (u32 ThreadArenaIndex = 0;
                ThreadArenaIndex < LimitThreadArenas;
                ++ThreadArenaIndex)
    {
        thread_memory_arena * TestThreadArena = ThreadArenaArray + ThreadArenaIndex;
        if (
                !TestThreadArena->InUse && 
                (MinDesiredSize == 0 || TestThreadArena->Arena.MaxSize >= MinDesiredSize)
            )
        {
            TestThreadArena->InUse = true;
            ThreadArena = TestThreadArena;
            break;
        }
    }

    return ThreadArena;
}

void
Memset(u8 * Dest,u8 c,u32 Size)
{
    memset(Dest,(int)c,Size);
}

void
MemCopy(u8 * Dest,u8 * Src,u32 EntitySize)
{
    Assert(Dest);
    Assert(Src);
    Assert(EntitySize > 0);
    for (u32 b = 0;
            b < EntitySize;
            ++b)
    {
        Dest[b] = Src[b];
    }
}

memory_aligned_result
AlignMemoryAddress(void * Addr,u32 Align)
{
    memory_aligned_result Rslt = {};
    uintptr_t AddrBegin = (uintptr_t)Addr;
    Rslt.AddressAligned = (AddrBegin + (uintptr_t)Align) & ~(uintptr_t)Align;
    Rslt.Delta          = (u32)(Rslt.AddressAligned - AddrBegin);

    return Rslt;
}

