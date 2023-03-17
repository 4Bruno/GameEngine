/*
 * This is internal header for vulkan to be shared among vulkan files
 * I have split them to handle initialization and general pipelines, ground and helpers.
 * vulkan_initializer.cpp is in charge of importing vulkan lib functions and setting globals
 */
#ifndef VULKAN_LOCAL_H
#define VULKAN_LOCAL_H
#include "graphics_api.h"
// TODO: memcpy. How can we avoid this?
#include <memory.h>
#include "hierarchy_tree.h"



VkDescriptorSetLayoutBinding
VH_CreateDescriptorSetLayoutBinding(u32 BindingSlot,VkDescriptorType DescriptorType,VkShaderStageFlags ShaderStageFlags);

b32
VH_GetSupportedDepthFormat(VkPhysicalDevice PhysicalDevice, VkFormat *DepthFormat, b32 UseHighestPrecision = true);

void
VH_TranstionTo(VkCommandBuffer cmdBuffer, vulkan_image * VulkanImage, VkImageLayout   dstLayout, VkAccessFlags   dstAccesses);

inline void
VH_DeleteVulkanBuffer(vulkan_buffer * Buffer)
{
    if (VK_VALID_HANDLE(Buffer->Buffer))
    {
        vkDestroyBuffer(Buffer->DeviceAllocator,Buffer->Buffer,0);
    } 

    if (VK_VALID_HANDLE(Buffer->DeviceMemory))
    {
        vkFreeMemory(Buffer->DeviceAllocator, Buffer->DeviceMemory, 0);
    }
}

i32
VH_FindSuitableMemoryIndex(VkPhysicalDevice PhysicalDevice, VkMemoryRequirements MemoryRequirements,VkMemoryPropertyFlags PropertyFlags);
i32
VH_FindSuitableMemoryIndex(VkPhysicalDevice PhysicalDevice, VkMemoryRequirements2 MemoryRequirements,VkMemoryPropertyFlags PropertyFlags);

VkPipelineLayoutCreateInfo
VH_CreatePipelineLayoutCreateInfo();

VkPipelineShaderStageCreateInfo
VH_CreateShaderStageInfo(VkShaderStageFlagBits Stage, VkShaderModule Module);

VkPipelineVertexInputStateCreateInfo
VH_CreateVertexInputStateInfo();

VkPipelineInputAssemblyStateCreateInfo
VH_CreatePipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology Topology);

VkPipelineRasterizationStateCreateInfo
VH_CreatePipelineRasterizationStateCreateInfo(VkPolygonMode PolygonMode);

VkPipelineMultisampleStateCreateInfo 
VH_CreatePipelineMultisampleStateCreateInfo();

VkPipelineColorBlendAttachmentState
VH_CreatePipelineColorBlendAttachmentState();

VkDeviceSize
VH_PaddedUniformBuffer(VkDeviceSize Size);

VkDeviceSize
VH_PaddedStorageBuffer(VkDeviceSize Size);

i32
VH_CreateBuffer(VkPhysicalDevice PhysicalDevice,VkDevice Device, VkDeviceSize Size, VkSharingMode SharingMode, VkMemoryPropertyFlags PropertyFlags, VkBufferUsageFlags Usage, vulkan_buffer * Buffer, u32 SharedBufferQueueFamilyIndexCount = 0, u32 * SharedBufferQueueFamilyIndexArray = 0);

VkViewport
VH_CreateDefaultViewport(VkExtent2D WindowExtent);

VkPipeline
VH_PipelineBuilder(vulkan_pipeline * VulkanPipeline,VkDevice Device, VkRenderPass RenderPass, u32 Subpass = 0);

i32
VH_CreateCommandBuffers(VkDevice Device,VkCommandPool CommandPool,u32 CommandBufferCount,VkCommandBuffer * CommandBuffers);

i32
VH_CreateCommandPool(VkDevice Device, u32 QueueFamilyIndex, VkCommandPoolCreateFlags CommandPoolCreateFlags, VkCommandPool * CommandPool);

vulkan_image *
VH_CreateDepthBuffer(gpu_arena * Arena, VkExtent3D Extent);

VkImageCreateInfo
VH_CreateImageCreateInfo2D(VkExtent3D Extent, VkFormat Format, VkImageUsageFlags Usage);

VkImageCreateInfo
VH_DepthBufferCreateInfo(VkExtent3D Extent);

VkWriteDescriptorSet
VH_WriteDescriptor(u32 BindingSlot,VkDescriptorSet Set,VkDescriptorType DescriptorType, VkDescriptorBufferInfo * BufferInfo);
VkWriteDescriptorSet
VH_WriteDescriptor(u32 BindingSlot,VkDescriptorSet Set,VkDescriptorType DescriptorType, VkDescriptorImageInfo * DescriptorInfo);

i32
VH_AllocateDescriptor(VkDescriptorSetLayout  * SetLayout, VkDescriptorPool Pool, VkDescriptorSet * Set);

i32
VH_CopyBuffer(VkCommandBuffer CommandBuffer, VkBuffer Src, VkBuffer Dest, VkDeviceSize Size, VkDeviceSize Offset);


i32
VH_BeginCommandBuffer(VkCommandBuffer CommandBuffer);
i32
VH_EndCommandBuffer(VkCommandBuffer CommandBuffer, VkQueue FamilyQueue);

void
VH_DestroyImage(VkDevice Device,vulkan_image * Image);

i32
VH_PushVertexData(void * Data, u32 DataSize, u32 BaseOffset);

gpu_arena
VH_AllocateMemory(VkPhysicalDevice PhysicalDevice, VkDevice Device, VkMemoryRequirements MemReq, VkMemoryPropertyFlags MemPropFlags);
VkResult
VH_BindBufferToArena(VkDevice Device,gpu_arena * Arena,vulkan_buffer * VulkanBuffer);
VkResult
VH_BindImageToArena(VkDevice Device,gpu_arena * Arena,VkImage Image);
i32
VH_CreateUnAllocArenaImage(VkPhysicalDevice PhysicalDevice,
                           VkDevice Device, u32 Size,
                           gpu_arena * Arena);

i32
VH_CreateUnAllocArenaBuffer(VkPhysicalDevice PhysicalDevice,
                      VkDevice Device, VkDeviceSize Size, 
                      VkSharingMode SharingMode, VkMemoryPropertyFlags PropertyFlags, VkBufferUsageFlags Usage,
                      gpu_arena * Arena,
                      u32 SharedBufferQueueFamilyIndexCount = 0,
                      u32 * SharedBufferQueueFamilyIndexArray = 0);

#endif
