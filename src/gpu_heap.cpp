#include "gpu_heap.h"
#include "game_math.h"
#include "vulkan_initializer.h"

#define MIN_GPU_HEAP_HEADER_BLOCKS  1
#define MAX_GPU_HEAP_HEADER_BLOCKS  200

u32
AlignDataSize(u32 DataSize, u32 Alignment)
{
    u32 Align       = Alignment - 1;
    u32 AlignedData = ((u32)DataSize + Align) & ~Align;

    return AlignedData;
}

internal b32
HeapBlockSizeAlignsWithGPUMemoryReq(gpu_heap * Heap)
{
    u32 BlockSizeMinusOne = (Heap->BlockMinSize - 1);
    // request is not asking for a memory alignment bigger than min block size
    b32 Result = ( ((Heap->Alignment + BlockSizeMinusOne) & ~BlockSizeMinusOne) == Heap->BlockMinSize);

    return Result;
}

i32
InitializeUnallocHeapBuffer(
                            gpu_heap              * Heap,
                            memory_arena          * ParentArenaToAllocHeaders,
                            VkPhysicalDevice        PhysicalDevice,
                            VkDevice                Device,
                            VkDeviceSize            RequestedMemorySize,
                            u32                     RequestedMinBlockSize,
                            VkImageCreateInfo     * CreateInfo, 
                            VkImageAspectFlagBits   PlaneAspect,
                            VkMemoryPropertyFlags   MemoryPropertyFlags
                           )
{

    VkMemoryRequirements2 MemoryRequirements = {};
    MemoryRequirements.sType              = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2; // sType   sType
    MemoryRequirements.pNext              = 0; // Void * pNext
    MemoryRequirements.memoryRequirements = {}; // memoryRequirements   memoryRequirements
    
    VkDeviceImageMemoryRequirements ReqImage;
    ReqImage.sType       = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS; // sType   sType
    ReqImage.pNext       = 0; // Void * pNext
    ReqImage.pCreateInfo = CreateInfo; // VkImageCreateInfo * pCreateInfo
    ReqImage.planeAspect = PlaneAspect; // planeAspect   planeAspect

    vkGetDeviceImageMemoryRequirements(Device,&ReqImage,&MemoryRequirements);
    VkMemoryPropertyFlags ImageMemoryPropertyFlags = MemoryPropertyFlags;
    i32 MemoryTypeIndex = VH_FindSuitableMemoryIndex(PhysicalDevice,MemoryRequirements,ImageMemoryPropertyFlags);

    // This is dirty code
    // if you dont pass memory size assume this arena is for a single allocation
    // and should match the size of the image
    // depth buffer and other output images
    u32 MaxHeadersSize = sizeof(gpu_heap_block);
    u32 AlignedMemSize = (u32)MemoryRequirements.memoryRequirements.size;
    if (RequestedMemorySize > 0)
    {
        Assert(NextPowerOf2(RequestedMinBlockSize) == RequestedMinBlockSize);
        AlignedMemSize = (u32)RequestedMemorySize;
    }

    AlignedMemSize  = AlignDataSize(AlignedMemSize, RequestedMinBlockSize);
    u32 MaxHeadersCount = minval(
                                    maxval( CeilDivide(AlignedMemSize, RequestedMinBlockSize) , MIN_GPU_HEAP_HEADER_BLOCKS), 
                                    MAX_GPU_HEAP_HEADER_BLOCKS);

    // TODO: is this realistic? worst case scenario
    MaxHeadersSize  *= MaxHeadersCount;
    Logn("GPU Heap allocator with worst case headers %i (KB: %f)", MaxHeadersCount, (r32)MaxHeadersSize / 1024.0f);

    // This is invalid assertion as the passed image view is just to query the alignment req
    // for the images that are supposed to be storaged in this heap
    // Assert(MemoryRequirements.memoryRequirements.size == AlignedMemSize);

    Heap->Type = gpu_arena_type_image;
    
    Heap->Blocks               = 0;
    Heap->FreeBlocks           = 0;
    Heap->BlockCount           = 0;
    Heap->BlockMinSize         = RequestedMinBlockSize;
    Heap->MaxSize              = AlignedMemSize; // MaxSize   MaxSize
    Heap->Alignment            = (u32)MemoryRequirements.memoryRequirements.alignment;
    Heap->HeapBlockHeaderArena = AllocateSubArena(ParentArenaToAllocHeaders, MaxHeadersSize);
    
    Heap->GPU             = PhysicalDevice;
    Heap->Device          = Device;
    Heap->Buffer          = VK_NULL_HANDLE; 
    Heap->MemoryIndexType = MemoryTypeIndex;

    Heap->WriteToAddr     = 0; 

    Assert( HeapBlockSizeAlignsWithGPUMemoryReq(Heap) );

    return 0;
}

i32
InitializeUnallocHeapBuffer(
                        gpu_heap              * Heap, 
                        memory_arena          * ParentArenaToAllocHeaders,
                        VkPhysicalDevice        PhysicalDevice,
                        VkDevice                Device, 
                        VkDeviceSize            RequestedMemorySize, 
                        u32                     RequestedMinBlockSize,
                        VkSharingMode           SharingMode, 
                        VkMemoryPropertyFlags   PropertyFlags, 
                        VkBufferUsageFlags      Usage,
                        u32                     SharedBufferQueueFamilyIndexCount,
                        u32                   * SharedBufferQueueFamilyIndexArray
)
{
    Assert(NextPowerOf2(RequestedMinBlockSize) == RequestedMinBlockSize);

    u32 AlignedMemSize  = AlignDataSize((u32)RequestedMemorySize, RequestedMinBlockSize);
    u32 MaxHeadersCount = minval(
                                    maxval( CeilDivide(AlignedMemSize, RequestedMinBlockSize) , MIN_GPU_HEAP_HEADER_BLOCKS), 
                                    MAX_GPU_HEAP_HEADER_BLOCKS);

    // TODO: is this realistic? worst case scenario
    u32 MaxHeadersSize  = MaxHeadersCount * sizeof(gpu_heap_block);
    Logn("GPU Heap allocator with worst case headers %i (KB: %f)", MaxHeadersCount, (r32)MaxHeadersSize / 1024.0f);

    VkBufferCreateInfo BufferCreateInfo;

    BufferCreateInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO; // VkStructureType sType;
    BufferCreateInfo.pNext       = 0;           // Void * pNext;
    BufferCreateInfo.flags       = 0;           // VkBufferCreateFlags flags;
    BufferCreateInfo.size        = AlignedMemSize;        // VkDeviceSize size;
    BufferCreateInfo.usage       = Usage;       // VkBufferUsageFlags usage;
    BufferCreateInfo.sharingMode = SharingMode; // VkSharingMode sharingMode;

    if ( SharingMode == VK_SHARING_MODE_CONCURRENT )
    {
        if ( SharedBufferQueueFamilyIndexCount == 0 )
        {
            Log("Error buffer creation. Shared buffer requires family queue indexes and count\n");
            return 1;
        }
        BufferCreateInfo.queueFamilyIndexCount = SharedBufferQueueFamilyIndexCount; // u32_t queueFamilyIndexCount;
        BufferCreateInfo.pQueueFamilyIndices   = SharedBufferQueueFamilyIndexArray; // Typedef * pQueueFamilyIndices;
    }
    else
    {
        BufferCreateInfo.queueFamilyIndexCount = 0;           // u32_t queueFamilyIndexCount;
        BufferCreateInfo.pQueueFamilyIndices   = 0;           // Typedef * pQueueFamilyIndices;
    }

    VK_CHECK(vkCreateBuffer(Device, &BufferCreateInfo,0, &Heap->Buffer));

    VkMemoryRequirements MemoryRequirements;
    vkGetBufferMemoryRequirements(Device, Heap->Buffer, &MemoryRequirements);
    i32 MemoryTypeIndex = VH_FindSuitableMemoryIndex(PhysicalDevice,MemoryRequirements,PropertyFlags);
    
    Assert(MemoryRequirements.size == AlignedMemSize);

    Heap->Blocks               = 0;
    Heap->FreeBlocks           = 0;
    Heap->BlockMinSize         = RequestedMinBlockSize;
    Heap->HeapBlockHeaderArena = AllocateSubArena(ParentArenaToAllocHeaders, MaxHeadersSize);

    Heap->MaxSize              = (u32)MemoryRequirements.size;
    Heap->Alignment            = (u32)MemoryRequirements.alignment;

    Heap->MemoryIndexType      = MemoryTypeIndex;
    Heap->Device               = Device; 
    Heap->Type                 = gpu_arena_type_buffer;
    Heap->GPU                  = PhysicalDevice;

    Heap->WriteToAddr = 0;

    Assert( HeapBlockSizeAlignsWithGPUMemoryReq(Heap) );

    return 0;
}

gpu_heap_block *
GetFreeBlockOrAlloc(gpu_heap * Heap)
{
    gpu_heap_block * Block = Heap->FreeBlocks;

    if (!Block)
    {
        Block = PushStruct(Heap->HeapBlockHeaderArena, gpu_heap_block);
    }
    else
    {
        Heap->FreeBlocks       = Block->Next;
        Heap->FreeBlocks->Prev = 0;
    }

    RtlZeroMemory(Block, sizeof(gpu_heap_block));

    Assert(Block); // impossible

    return Block;
}

void
AppendToFrontList(gpu_heap * Heap, gpu_heap_block * Block)
{
    Block->Next = Heap->FreeBlocks;
    Block->Prev = 0;
    Heap->FreeBlocks = Block;
    --Heap->BlockCount;
}

void
ReleaseBlock(gpu_heap * Heap, gpu_heap_block * Block)
{
    Block->InUse = false;
    while (Block->Next && !Block->Next->InUse)
    {
        gpu_heap_block * BlockFreed = Block->Next;
        Block->Size += BlockFreed->Size;
        Block->Next = Block->Next->Next;

        AppendToFrontList(Heap, BlockFreed);
    }
    while (Block->Prev && !Block->Prev->InUse)
    {
        gpu_heap_block * Prev = Block->Prev;

        Prev->Size += Block->Size;
        Prev->Next = Block->Next;

        AppendToFrontList(Heap, Block);

        Block = Prev;
    }
}

gpu_heap_block *
FindHeapBlock(gpu_heap * Heap, u32 Size)
{
    u32 SizeAligned = AlignDataSize(Size, Heap->BlockMinSize);

    gpu_heap_block * Block     = 0;
    gpu_heap_block * PrevBlock = 0;

    for (   Block = Heap->Blocks;
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
        // Assert(PrevBlock);
        u64 OffsetAfterPrevBlock = 0;

        if (PrevBlock)
        {
            OffsetAfterPrevBlock = PrevBlock->Begin + PrevBlock->Size;
        }

        Assert(Heap->MaxSize >= (OffsetAfterPrevBlock + SizeAligned));

        Block = GetFreeBlockOrAlloc(Heap);

        Block->Begin = OffsetAfterPrevBlock;
        Block->InUse = true;
        Block->Prev  = PrevBlock;
        Block->Next  = 0;
        Block->Size  = SizeAligned;

        if (PrevBlock)
        {
            PrevBlock->Next = Block;
        }
    }
    else
    {
        // Can we split?
        u32 ExtraSize = (Block->Size - SizeAligned);
        if (ExtraSize > Heap->BlockMinSize)
        {
            u32 RemainingSize = (Block->Size - SizeAligned);
            Assert((RemainingSize % Heap->BlockMinSize) == 0);

            gpu_heap_block * BlockDivided = GetFreeBlockOrAlloc(Heap);

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

    Block->ID = ++Heap->BlockCount;

    return Block;
}



gpu_heap_block *
FindHeapBlockByID(gpu_heap * Heap, u32 ID)
{
    gpu_heap_block * Block = 0;

    for (   Block = Heap->Blocks;
            (Block);
            Block = Block->Next)
    {
        if (Block->ID == ID)
        {
            break;
        }
    }

    return Block;
}
