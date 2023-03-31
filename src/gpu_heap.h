#ifndef GAME_GPU_HEAP_H
#define GAME_GPU_HEAP_H

#include "game_platform.h"
#include "game_memory.h"

struct vulkan_image
{
    VkImage              Image;
    VkImageView          ImageView;
    VkFormat             Format;
    VkImageUsageFlags    UsageFlags;
    VkMemoryRequirements MemoryRequirements;
    VkImageLayout        CurrentLayout;
    VkImageAspectFlags   CurrentAccess;
};


enum gpu_arena_type
{
    gpu_arena_type_buffer,
    gpu_arena_type_image
};

struct gpu_heap_block
{
    gpu_heap_block * Next;
    gpu_heap_block * Prev;

    b32 InUse;
    u64 Begin;
    u32 Size;

    vulkan_image Image;

    u32 ID;
};

struct gpu_heap
{
    gpu_arena_type   Type;
    void * Owner;

    u64 MaxSize;
    u32 BlockMinSize; // must be power of 2

    memory_arena   * HeapBlockHeaderArena;
    gpu_heap_block * Blocks;
    u32              BlockCount;
    gpu_heap_block * FreeBlocks;

    // GPU Memory Specific Info
    i32                MemoryIndexType;
    VkPhysicalDevice   GPU;
    VkDevice           Device;
    VkBuffer           Buffer;
    u64                DeviceBindingOffsetBegin;
    u32                Alignment;

    // For pipe cpu-gpu
    void           * WriteToAddr;

};

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
                           );
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
                        u32                     SharedBufferQueueFamilyIndexCount = 0,
                        u32                   * SharedBufferQueueFamilyIndexArray = 0
        );

gpu_heap_block *
FindHeapBlock(gpu_heap * Heap, u32 Size);

void
ReleaseBlock(gpu_heap * Heap, gpu_heap_block * Block);

gpu_heap_block *
FindHeapBlockByID(gpu_heap * Heap, u32 ID);

#endif
