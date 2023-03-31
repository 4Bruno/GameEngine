#ifndef GAME_HEAP_H
#define GAME_HEAP_H

#include "game_platform.h"
#include "game_memory.h"

struct heap_block
{
    heap_block * Next;
    heap_block * Prev;

    b32 InUse;
    u64 Begin;

    // Always includes heap_block
    // header struct
    u32 Size;
};

struct heap
{
    void * Memory;
    u32 MaxSize;
    // must be power of 2
    u32 BlockMinSize;

    heap_block Blocks;
};

void
InitializeHeap(heap * Heap, void * MemoryAddr, u32 MemorySize, u32 MinBlockSize);

heap_block *
FindHeapBlock(heap * Heap, u32 Size);

void
ReleaseBlock(heap * Heap, heap_block * Block);

#endif
