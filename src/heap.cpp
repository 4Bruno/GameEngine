#include "heap.h"
#include "game_math.h"



void
InitializeHeap(heap * Heap, void * MemoryAddr, u32 MemorySize, u32 MinBlockSize)
{
    Heap->Memory = MemoryAddr;
    Heap->Blocks.Begin = 0;
    Heap->Blocks.InUse = true;
    Heap->Blocks.Prev = Heap->Blocks.Next = 0; // handles first iteration
    Assert(NextPowerOf2(MinBlockSize) == MinBlockSize);
    Heap->BlockMinSize = MinBlockSize;
    Heap->MaxSize = MemorySize;
}

void
ReleaseBlock(heap * Heap, heap_block * Block)
{
    Block->InUse = false;
    while (Block->Next && !Block->Next->InUse)
    {
        Block->Size += Block->Next->Size;
        Block->Next = Block->Next->Next;
    }
    while (Block->Prev && !Block->Prev->InUse)
    {
        heap_block * Prev = Block->Prev;

        Prev->Size += Block->Size;
        Prev->Next = Block->Next;

        Block = Prev;
    }
}

heap_block *
FindHeapBlock(heap * Heap, u32 Size)
{
    u32 SizeRequirements = Size + sizeof(heap_block);
    u32 Align            = Heap->BlockMinSize - 1;
    u32 SizeAligned      = (SizeRequirements + Align) & ~Align;

    heap_block * Block     = 0;
    heap_block * PrevBlock = 0;

    for (   Block = &Heap->Blocks;
            (Block);
            Block = Block->Next)
    {
        if (!Block->InUse && (Block->Size >= SizeAligned))
        {
            break;
        }

        if (Block->Next)
        {
            uintptr_t MemGap = (Block->Next->Begin - (Block->Begin + Block->Size));
            if (MemGap >= SizeAligned)
            {
                break;
            }
        }

        PrevBlock = Block;
    }

    if(!Block)
    {
        Assert(PrevBlock);

        u64 OffsetAfterPrevBlock = PrevBlock->Begin + PrevBlock->Size;
        Assert(Heap->MaxSize >= (OffsetAfterPrevBlock + SizeAligned));

        u8 * Addr = ((u8 *)Heap->Memory + OffsetAfterPrevBlock);
        Block = (heap_block *)Addr;
        Block->Begin = OffsetAfterPrevBlock;
        Block->InUse = true;
        Block->Prev  = PrevBlock;
        Block->Next  = 0;
        Block->Size  = SizeAligned;

        PrevBlock->Next = Block;
    }
    else
    {
        // Can we split?
        u32 ExtraSize = (Block->Size - SizeAligned);
        if (ExtraSize > Heap->BlockMinSize)
        {
            u32 RemainingSize = (Block->Size - SizeAligned);
            Assert((RemainingSize % Heap->BlockMinSize) == 0);

            heap_block * BlockDivided = (heap_block *)((u8 *)Heap->Memory + Block->Begin + SizeAligned);
            BlockDivided->Begin = Block->Begin + SizeAligned;
            BlockDivided->InUse = false;
            BlockDivided->Size  = RemainingSize;
            BlockDivided->Prev  = Block;
            BlockDivided->Next  = Block->Next;
            Block->Next         = BlockDivided;
        }
        Block->Size = SizeAligned;
        Block->InUse = true;
    }

    return Block;
}

