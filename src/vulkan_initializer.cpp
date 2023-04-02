/*
 * This library is meant to be OS independent
 * All OS calls are perform in ???_platform.cpp OS .exe
 *  - Library load and Instance proc address is handle
 *  - OS specific window creation
 * InitializeVulkan will receive opaque pointers to
 * the function/data which is to be called in order to
 * create OS specific window
 *
 * - VulkanOnWindowResize
 *   Must be called from the OS on window resize event
 */
#include "vulkan_initializer.h"
#include "game_memory.h"

const char * CTableDestructorDebugNames[] = 
{
    "vulkan_destructor_type_vkDestroyUnknown",
    "vulkan_destructor_type_vkDestroySurfaceKHR",
    "vulkan_destructor_type_vkDestroySwapchainKHR",
    "vulkan_destructor_type_vkDestroyInstance",
    "vulkan_destructor_type_vkDestroyCommandPool",
    "vulkan_destructor_type_vkDestroyShaderModule",
    "vulkan_destructor_type_vkDestroyDevice",
    "vulkan_destructor_type_vkDestroyFence",
    "vulkan_destructor_type_vkDestroySemaphore",
    "vulkan_destructor_type_vkDestroyRenderPass",
    "vulkan_destructor_type_vkDestroySampler",
    "vulkan_destructor_type_vkDestroyImageView",
    "vulkan_destructor_type_vkDestroyFramebuffer" ,
    "vulkan_destructor_type_vkDestroyPipeline",
    "vulkan_destructor_type_vkDestroyPipelineLayout",
    "vulkan_destructor_type_vkDestroyDescriptorSetLayout",
    "vulkan_destructor_type_vkDestroyDescriptorPool",
    "vulkan_destructor_type_vkDestroyBuffer",
    "vulkan_destructor_type_vkDestroyImage",
    "vulkan_destructor_type_vkDestroyDebugUtilsMessengerEXT",
    "vulkan_destructor_type_vkFreeCommandBuffers",

    "vulkan_destructor_type_vkDestroyArenaCustom",
    "vulkan_destructor_type_vkDestroyMemoryPoolCustom",
    "vulkan_destructor_type_vkDestroyHeapCustom",
};



PFN_vkGetInstanceProcAddr                     vkGetInstanceProcAddr                   = 0;

// INSTANCE VULKAN
PFN_vkCreateInstance                          vkCreateInstance                        = 0;
PFN_vkEnumerateInstanceExtensionProperties    vkEnumerateInstanceExtensionProperties  = 0;
PFN_vkEnumerateInstanceLayerProperties        vkEnumerateInstanceLayerProperties      = 0;


// PHYSICAL DEVICES
// SWAP CHAIN API
PFN_vkDestroySurfaceKHR                       vkDestroySurfaceKHR                       = 0;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR      vkGetPhysicalDeviceSurfaceSupportKHR      = 0;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = 0;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR      vkGetPhysicalDeviceSurfaceFormatsKHR      = 0;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = 0;
PFN_vkGetPhysicalDeviceMemoryProperties       vkGetPhysicalDeviceMemoryProperties       = 0;
PFN_vkGetPhysicalDeviceFormatProperties       vkGetPhysicalDeviceFormatProperties       = 0;

PFN_vkCreateSwapchainKHR                      vkCreateSwapchainKHR                      = 0;
PFN_vkDestroySwapchainKHR                     vkDestroySwapchainKHR                     = 0;
PFN_vkGetSwapchainImagesKHR                   vkGetSwapchainImagesKHR                   = 0;
PFN_vkAcquireNextImageKHR                     vkAcquireNextImageKHR                     = 0;
PFN_vkQueuePresentKHR                         vkQueuePresentKHR                         = 0;


PFN_vkEnumeratePhysicalDevices                vkEnumeratePhysicalDevices                = 0;
PFN_vkEnumerateDeviceExtensionProperties      vkEnumerateDeviceExtensionProperties      = 0;
PFN_vkGetPhysicalDeviceProperties             vkGetPhysicalDeviceProperties             = 0;
PFN_vkGetPhysicalDeviceFeatures               vkGetPhysicalDeviceFeatures               = 0;
PFN_vkGetPhysicalDeviceFeatures2              vkGetPhysicalDeviceFeatures2              = 0;
PFN_vkGetPhysicalDeviceQueueFamilyProperties  vkGetPhysicalDeviceQueueFamilyProperties  = 0;
PFN_vkCreateDevice                            vkCreateDevice                            = 0;
PFN_vkGetDeviceProcAddr                       vkGetDeviceProcAddr                       = 0;
PFN_vkDestroyInstance                         vkDestroyInstance                         = 0;
PFN_vkCreateCommandPool                       vkCreateCommandPool                       = 0;
PFN_vkDestroyCommandPool                      vkDestroyCommandPool                      = 0;
PFN_vkResetCommandPool                        vkResetCommandPool                        = 0;
PFN_vkResetCommandBuffer                      vkResetCommandBuffer                      = 0;
PFN_vkAllocateCommandBuffers                  vkAllocateCommandBuffers                  = 0;  
PFN_vkFreeCommandBuffers                      vkFreeCommandBuffers                      = 0;  
PFN_vkBeginCommandBuffer                      vkBeginCommandBuffer                      = 0;
PFN_vkEndCommandBuffer                        vkEndCommandBuffer                        = 0;
PFN_vkCmdPushConstants                        vkCmdPushConstants                        = 0;
PFN_vkCmdCopyBuffer                           vkCmdCopyBuffer                           = 0;
PFN_vkCmdCopyBufferToImage                    vkCmdCopyBufferToImage                    = 0;
PFN_vkCmdBeginRenderPass                      vkCmdBeginRenderPass                      = 0;
PFN_vkCmdEndRenderPass                        vkCmdEndRenderPass                        = 0;
PFN_vkCmdPipelineBarrier                      vkCmdPipelineBarrier                      = 0;
PFN_vkCmdClearColorImage                      vkCmdClearColorImage                      = 0;
PFN_vkCmdBindPipeline                         vkCmdBindPipeline                         = 0;
PFN_vkCmdDraw                                 vkCmdDraw                                 = 0;
PFN_vkCmdDrawIndexed                          vkCmdDrawIndexed                          = 0;
PFN_vkCmdBindVertexBuffers                    vkCmdBindVertexBuffers                    = 0;
PFN_vkCmdBindIndexBuffer                      vkCmdBindIndexBuffer                      = 0;
PFN_vkQueueSubmit                             vkQueueSubmit                             = 0;
PFN_vkQueueWaitIdle                           vkQueueWaitIdle                           = 0;
PFN_vkCreateShaderModule                      vkCreateShaderModule                      = 0;
PFN_vkDestroyShaderModule                     vkDestroyShaderModule                     = 0;
PFN_vkCreateDebugUtilsMessengerEXT            vkCreateDebugUtilsMessengerEXT            = 0;
PFN_vkDestroyDebugUtilsMessengerEXT           vkDestroyDebugUtilsMessengerEXT           = 0;
PFN_vkSetDebugUtilsObjectNameEXT              vkSetDebugUtilsObjectNameEXT              = 0;

//  LOGICAL DEVICE
PFN_vkGetDeviceQueue                          vkGetDeviceQueue                          = 0;
PFN_vkDestroyDevice                           vkDestroyDevice                           = 0;
PFN_vkDeviceWaitIdle                          vkDeviceWaitIdle                          = 0;
PFN_vkCreateFence                             vkCreateFence                             = 0;
PFN_vkDestroyFence                            vkDestroyFence                            = 0;
PFN_vkWaitForFences                           vkWaitForFences                           = 0;
PFN_vkResetFences                             vkResetFences                             = 0;
PFN_vkCreateSemaphore                         vkCreateSemaphore                         = 0;
PFN_vkDestroySemaphore                        vkDestroySemaphore                        = 0;
PFN_vkCreateRenderPass                        vkCreateRenderPass                        = 0;
PFN_vkDestroyRenderPass                       vkDestroyRenderPass                       = 0;
PFN_vkCreateImageView                         vkCreateImageView                         = 0;
PFN_vkCreateImage                             vkCreateImage                             = 0;
PFN_vkCreateSampler                           vkCreateSampler                           = 0;
PFN_vkDestroySampler                          vkDestroySampler                          = 0;
PFN_vkDestroyImageView                        vkDestroyImageView                        = 0;
PFN_vkCreateFramebuffer                       vkCreateFramebuffer                       = 0; 
PFN_vkDestroyFramebuffer                      vkDestroyFramebuffer                      = 0; 
PFN_vkCreateGraphicsPipelines                 vkCreateGraphicsPipelines                 = 0;
PFN_vkCreatePipelineLayout                    vkCreatePipelineLayout                    = 0;
PFN_vkDestroyPipeline                         vkDestroyPipeline                         = 0;
PFN_vkDestroyPipelineLayout                   vkDestroyPipelineLayout                   = 0;

PFN_vkCreateDescriptorSetLayout               vkCreateDescriptorSetLayout               = 0;
PFN_vkDestroyDescriptorSetLayout              vkDestroyDescriptorSetLayout              = 0;
PFN_vkAllocateDescriptorSets                  vkAllocateDescriptorSets                  = 0;  
PFN_vkUpdateDescriptorSets                    vkUpdateDescriptorSets                    = 0;
PFN_vkCreateDescriptorPool                    vkCreateDescriptorPool                    = 0;
PFN_vkDestroyDescriptorPool                   vkDestroyDescriptorPool                   = 0;
PFN_vkCmdBindDescriptorSets                   vkCmdBindDescriptorSets                   = 0;
PFN_vkCmdSetViewport                          vkCmdSetViewport                          = 0;
PFN_vkCmdSetScissor                           vkCmdSetScissor                           = 0;

PFN_vkAllocateMemory                          vkAllocateMemory                          = 0;
PFN_vkFreeMemory                              vkFreeMemory                              = 0;
PFN_vkCreateBuffer                            vkCreateBuffer                            = 0;
PFN_vkDestroyBuffer                           vkDestroyBuffer                           = 0;
PFN_vkDestroyImage                            vkDestroyImage                            = 0;
PFN_vkGetBufferMemoryRequirements             vkGetBufferMemoryRequirements             = 0;
PFN_vkGetDeviceImageMemoryRequirements        vkGetDeviceImageMemoryRequirements        = 0;
PFN_vkGetImageMemoryRequirements              vkGetImageMemoryRequirements              = 0;
PFN_vkMapMemory                               vkMapMemory                               = 0;
PFN_vkUnmapMemory                             vkUnmapMemory                             = 0;
PFN_vkBindBufferMemory                        vkBindBufferMemory                        = 0;
PFN_vkBindImageMemory                         vkBindImageMemory                         = 0;

PFN_vkCmdSetDepthTestEnable                   vkCmdSetDepthTestEnable                   = 0;
PFN_vkCmdNextSubpass                          vkCmdNextSubpass                          = 0;

// GLOBALS
void
InitializeVulkanStruct(vulkan * V)
{
    V->MemoryArenasLabel        = "MEM ARENAS";
    V->MemoryHeapLabel          = "MEM HEAPS";
    V->DeviceMemoryPoolsLabel   = "MEMORY POOLS";
    V->StagingBufferLabel       = "STAGING BUFFERS";
    V->RenderPassLabel          = "RENDER PASSES";
    V->DescriptorSetLayoutLabel = "DESCRIPTOR SET LAYOUTS";
    V->FrameDataLabel           = "FRAME DATA";
    V->PipelinesLabel           = "PIPELINES";
    V->ShaderModulesLabel       = "SHADER MODULE";
}

vulkan                        GlobalVulkan = {};
b32                           GlobalWindowIsMinimized     = false;
vulkan_debug_obj_name_cache   DebugNamesCache             = {};

inline VkDeviceMemory
GetDeviceMemory(i32 MemoryTypeIndex)
{
    Assert(MemoryTypeIndex >= 0 && MemoryTypeIndex <= (i32)VK_MAX_MEMORY_TYPES);
    device_memory_pool * Pool = GlobalVulkan.DeviceMemoryPools.DeviceMemoryPool + MemoryTypeIndex;
    Assert(VK_VALID_HANDLE(Pool->DeviceMemory));
    Assert(Pool->Size > 0);
    VkDeviceMemory DeviceMemory = Pool->DeviceMemory; 


    return DeviceMemory;
};

inline void
CopyStr(char * DestStr,const char * SrcStr ,u32 Length)
{
    for (u32 ci = 0;
                ci < Length;
                ++ci)
    {
        DestStr[ci] = SrcStr[ci];
    }
}

int
strcmp(const char *p1, const char *p2)
{
    const unsigned char *s1 = (const unsigned char *) p1;
    const unsigned char *s2 = (const unsigned char *) p2;
    unsigned char c1, c2;
    do
    {
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;
        if (c1 == '\0')
            return c1 - c2;
    }
    while (c1 == c2);
    return c1 - c2;
}

#define VULKAN_TREE_APPEND_DATA(Type, Parent, Owner, Data) VulkanTreeAppend(vulkan_destructor_type_##Type, Parent, Owner, &Owner, Data)
#define VULKAN_TREE_APPEND(Type, Parent, Owner) VulkanTreeAppend(vulkan_destructor_type_##Type, (const void*)(Parent), (Owner), &(Owner))
#define VULKAN_TREE_APPEND_WITH_ID(Type, Parent, ID, Owner) VulkanTreeAppend(vulkan_destructor_type_##Type, (const void*)(Parent), (const void *)(ID), (Owner))
void
VulkanTreeAppend(vulkan_destructor_type DestructorType, const void * ParentID, const void * ID, const void * Owner, void * Data = 0)
{
    vulkan_node * Node = PushStruct(&GlobalVulkan.HTree->Arena, vulkan_node);
    Node->Data = Data;
    Node->ID = (void *)ID;
    Node->Owner = (void *)Owner;
    Node->DestructorType = DestructorType;
    HierarchyTreeAdd(GlobalVulkan.HTree, (void *)ParentID, Node);
}

#if DEBUG
void
SetDebugName(uint64_t Handle, const char * Name, VkObjectType ObjectType)
{
    Assert(DebugNamesCache.Count < ArrayCount(DebugNamesCache.Objects));
    Assert(Handle > 0);

    for (u32 i = 0; i < ArrayCount(DebugNamesCache.Objects);++i)
    {
        if (!DebugNamesCache.Objects[i].InUse)
        {
            CopyStr(DebugNamesCache.Objects[i].Name, Name, ArrayCount(DebugNamesCache.Objects[i].Name));
            DebugNamesCache.Objects[i].InUse = true;
            ++DebugNamesCache.Count;

            VkDebugUtilsObjectNameInfoEXT ObjectNameDebugInfo;
            ObjectNameDebugInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT; // sType   sType
            ObjectNameDebugInfo.pNext        = 0; // Void * pNext
            ObjectNameDebugInfo.objectType   = ObjectType; // objectType   objectType
            ObjectNameDebugInfo.objectHandle = (uint64_t)Handle; // objectHandle   objectHandle
            ObjectNameDebugInfo.pObjectName  = DebugNamesCache.Objects[i].Name; // Char_S * pObjectName

            vkSetDebugUtilsObjectNameEXT(GlobalVulkan.PrimaryDevice,&ObjectNameDebugInfo);

            break;
        }
    }
}
#else
void
SetDebugName(uintptr_t Handle, const char * Name, VkObjectType ObjectType);
#endif

#if 1
void SetDebugName(VkBuffer object, const char *  name)                  { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_BUFFER); }
void SetDebugName(VkCommandBuffer object, const char *  name)           { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_COMMAND_BUFFER ); }
void SetDebugName(VkCommandPool object, const char *  name)             { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_COMMAND_POOL ); }
void SetDebugName(VkBufferView object, const char *  name)              { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_BUFFER_VIEW); }
void SetDebugName(VkDescriptorPool object, const char *  name)          { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_DESCRIPTOR_POOL); }
void SetDebugName(VkDescriptorSet object, const char *  name)           { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_DESCRIPTOR_SET); }
void SetDebugName(VkDescriptorSetLayout object, const char *  name)     { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT); }
void SetDebugName(VkDevice object, const char *  name)                  { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_DEVICE); }
void SetDebugName(VkDeviceMemory object, const char *  name)            { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_DEVICE_MEMORY); }
void SetDebugName(VkFramebuffer object, const char *  name)             { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_FRAMEBUFFER); }
void SetDebugName(VkImage object, const char *  name)                   { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_IMAGE); }
void SetDebugName(VkImageView object, const char *  name)               { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_IMAGE_VIEW); }
void SetDebugName(VkPipeline object, const char *  name)                { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_PIPELINE); }
void SetDebugName(VkPipelineLayout object, const char *  name)          { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_PIPELINE_LAYOUT); }
void SetDebugName(VkQueryPool object, const char *  name)               { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_QUERY_POOL); }
void SetDebugName(VkQueue object, const char *  name)                   { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_QUEUE); }
void SetDebugName(VkRenderPass object, const char *  name)              { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_RENDER_PASS); }
void SetDebugName(VkSampler object, const char *  name)                 { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_SAMPLER); }
void SetDebugName(VkSemaphore object, const char *  name)               { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_SEMAPHORE); }
void SetDebugName(VkShaderModule object, const char *  name)            { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_SHADER_MODULE); }
void SetDebugName(VkSwapchainKHR object, const char *  name)            { SetDebugName((uint64_t)object, name, VK_OBJECT_TYPE_SWAPCHAIN_KHR); }
#endif

VkImageCreateInfo
VH_CreateImageCreateInfo2D(VkExtent3D Extent, VkFormat Format, VkImageUsageFlags Usage)
{
    VkImageCreateInfo ImageCreateInfo;

    ImageCreateInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO; // VkStructureType   sType;
    ImageCreateInfo.pNext                 = 0;                         // Void * pNext;
    ImageCreateInfo.flags                 = 0;                         // VkImageCreateFlags flags;
    ImageCreateInfo.imageType             = VK_IMAGE_TYPE_2D;          // VkImageType imageType;
    ImageCreateInfo.format                = Format;                    // VkFormat format;
    ImageCreateInfo.extent                = Extent;                    // VkExtent3D extent;
    ImageCreateInfo.mipLevels             = 1;                         // u32_t mipLevels;
    ImageCreateInfo.arrayLayers           = 1;                         // u32_t arrayLayers;
    ImageCreateInfo.samples               = VK_SAMPLE_COUNT_1_BIT;     // VkSampleCountFlagBits samples;
    ImageCreateInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;   // VkImageTiling tiling;
    ImageCreateInfo.usage                 = Usage;                     // VkImageUsageFlags usage;
    ImageCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE; // VkSharingMode sharingMode;
    ImageCreateInfo.queueFamilyIndexCount = 0;                         // u32_t queueFamilyIndexCount;
    ImageCreateInfo.pQueueFamilyIndices   = 0;                         // Typedef * pQueueFamilyIndices;
    ImageCreateInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED; // VkImageLayout initialLayout;

    return ImageCreateInfo;
}

i32
VH_FindSuitableMemoryIndex(VkPhysicalDevice PhysicalDevice, VkMemoryRequirements MemoryRequirements,VkMemoryPropertyFlags PropertyFlags)
{
    VkPhysicalDeviceMemoryProperties MemProps;
    vkGetPhysicalDeviceMemoryProperties(PhysicalDevice,&MemProps);

    for (u32 MemoryIndex = 0;
         MemoryIndex < MemProps.memoryTypeCount;
         ++MemoryIndex)
    {
        if (  
                ((1 << MemoryIndex) & MemoryRequirements.memoryTypeBits) &&
                ((MemProps.memoryTypes[MemoryIndex].propertyFlags & PropertyFlags) == PropertyFlags)
            )
        {
            return (i32)MemoryIndex;
        }
    }

    return -1;
}
i32
VH_FindSuitableMemoryIndex(VkPhysicalDevice PhysicalDevice, VkMemoryRequirements2 MemoryRequirements,VkMemoryPropertyFlags PropertyFlags)
{
    return VH_FindSuitableMemoryIndex(PhysicalDevice,MemoryRequirements.memoryRequirements,PropertyFlags);
}

void
VH_DestroyImage(VkDevice Device, vulkan_image * Image)
{
    if ( VK_VALID_HANDLE(Image->Image) )
    {
        vkDestroyImage(Device,Image->Image,0);
    }
    if ( VK_VALID_HANDLE(Image->ImageView) )
    {
        vkDestroyImageView(Device,Image->ImageView,0);
    }

}


vulkan_image *
VH_CreateImage(gpu_arena * Arena, VkFormat Format, VkImageUsageFlags UsageFlags, VkExtent3D Extent)
{
    Assert(Arena->Type == gpu_arena_type_image);
    Assert(Arena->ImageCount < ArrayCount(Arena->Images));

    vulkan_image * VulkanImage = Arena->Images + Arena->ImageCount;
    VkDeviceMemory DeviceMemory = GetDeviceMemory(Arena->MemoryIndexType);

    VkImageCreateInfo ImageCreateInfo = 
        VH_CreateImageCreateInfo2D(Extent, Format, UsageFlags);

    if (VK_FAILS(vkCreateImage(Arena->Device,&ImageCreateInfo, 0, &VulkanImage->Image)))
    {
        return 0;
    }

    vkGetImageMemoryRequirements(Arena->Device, VulkanImage->Image, &VulkanImage->MemoryRequirements);
    VkMemoryPropertyFlags PropertyFlags = VK_MEMORY_GPU;
    i32 MemoryTypeIndex = VH_FindSuitableMemoryIndex(GlobalVulkan.PrimaryGPU, VulkanImage->MemoryRequirements, PropertyFlags);

    // only 1 type for now
    Assert(MemoryTypeIndex == Arena->MemoryIndexType);

    VkDeviceSize BindMemoryOffset = Arena->DeviceBindingOffsetBegin + Arena->CurrentSize;

    if (VK_FAILS(vkBindImageMemory(Arena->Device, VulkanImage->Image,DeviceMemory, BindMemoryOffset)))
    {
        VH_DestroyImage(Arena->Device,VulkanImage);
        return 0;
    }

    VkImageViewCreateInfo ImageViewCreateInfo;

    ImageViewCreateInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; // VkStructureType   sType;
    ImageViewCreateInfo.pNext            = 0; // Void * pNext;
    ImageViewCreateInfo.flags            = 0; // VkImageViewCreateFlags   flags;
    ImageViewCreateInfo.image            = VulkanImage->Image; // VkImage   image;
    ImageViewCreateInfo.viewType         = VK_IMAGE_VIEW_TYPE_2D; // VkImageViewType   viewType;
    ImageViewCreateInfo.format           = Format; // VkFormat   format;

    VkComponentMapping   components;
    components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // VkComponentSwizzle   r;
    components.g = VK_COMPONENT_SWIZZLE_IDENTITY; // VkComponentSwizzle   g;
    components.b = VK_COMPONENT_SWIZZLE_IDENTITY; // VkComponentSwizzle   b;
    components.a = VK_COMPONENT_SWIZZLE_IDENTITY; // VkComponentSwizzle   a;
    ImageViewCreateInfo.components       = components; // VkComponentMapping   components;

    VkImageSubresourceRange   subresourceRange;
    subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT; // VkImageAspectFlags   aspectMask;
    subresourceRange.baseMipLevel   = 0; // uint32_t   baseMipLevel;
    subresourceRange.levelCount     = 1; // uint32_t   levelCount;
    subresourceRange.baseArrayLayer = 0; // uint32_t   baseArrayLayer;
    subresourceRange.layerCount     = 1; // uint32_t   layerCount;

    ImageViewCreateInfo.subresourceRange = subresourceRange; // VkImageSubresourceRange   subresourceRange;

    if (VK_FAILS(vkCreateImageView( GlobalVulkan.PrimaryDevice, &ImageViewCreateInfo, 0, &VulkanImage->ImageView)))
    {
        vkDestroyImage(Arena->Device,VulkanImage->Image,0);
        return 0;
    }

    ++Arena->ImageCount;
    VulkanImage->UsageFlags = UsageFlags;
    VulkanImage->Format         = Format;

    return VulkanImage;
}

i32
VH_CreateImage(VkDevice Device, u32 Width, u32 Height, u32 Channels, vulkan_image * Image)
{

    VkExtent3D MaxExtent3D = { Width, Height, 1};
    VkImageUsageFlags UsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat Format = VK_FORMAT_UNDEFINED;

    if (Channels == 3)
    {
        Format = VK_FORMAT_R8G8B8_SRGB; // VkFormat   format;
    }
    else if (Channels == 4)
    {
        Format = VK_FORMAT_R8G8B8A8_SRGB;
    }
    else if (Channels == 1)
    {
        Format = VK_FORMAT_R8_SRGB;
    }
    else
    {
        Assert(0);//INVALID_PATH_CODE;
    }

    VkImageCreateInfo ImageCreateInfo = 
        VH_CreateImageCreateInfo2D(MaxExtent3D, Format, UsageFlags);

    VK_CHECK(vkCreateImage(Device,&ImageCreateInfo, 0, &Image->Image));

    vkGetImageMemoryRequirements(Device, Image->Image, &Image->MemoryRequirements);
    VkMemoryPropertyFlags PropertyFlags = VK_MEMORY_GPU;
    i32 MemoryTypeIndex = VH_FindSuitableMemoryIndex(GlobalVulkan.PrimaryGPU, Image->MemoryRequirements, PropertyFlags);

    // only 1 type for now
    //Assert(MemoryTypeIndex == GlobalVulkan.TextureArena->MemoryIndexType);
    Assert(MemoryTypeIndex == GlobalVulkan.TextureHeap->MemoryIndexType);

    Image->Format = Format;
    Image->UsageFlags = UsageFlags;

    return 0;
}





struct vertex_inputs_description
{
    VkVertexInputBindingDescription   Bindings[1];
    VkVertexInputAttributeDescription Attributes[4];
};

vertex_inputs_description
RenderGetVertexInputsDescription()
{
    vertex_inputs_description InputsDescription;

    VkVertexInputBindingDescription Binding;

    Binding.binding   = 0;                           // u32_t binding;
    Binding.stride    = sizeof(vertex_point);        // u32_t stride;
    //Binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE; // VkVertexInputRate inputRate;
    Binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // VkVertexInputRate inputRate;

    VkVertexInputAttributeDescription * Attribute = (InputsDescription.Attributes + 0);

    Attribute->location = 0;                          // u32_t location;
    Attribute->binding  = 0;                          // u32_t binding;
    Attribute->format   = VK_FORMAT_R32G32B32_SFLOAT; // VkFormat format;
    Attribute->offset   = offsetof(vertex_point,P);   // u32_t offset;

    Attribute = (InputsDescription.Attributes + 1);
    
    Attribute->location = 1;                          // u32_t location;
    Attribute->binding  = 0;                          // u32_t binding;
    Attribute->format   = VK_FORMAT_R32G32B32_SFLOAT; // VkFormat format;
    Attribute->offset   = offsetof(vertex_point,N);   // u32_t offset;

    Attribute = (InputsDescription.Attributes + 2);

    Attribute->location = 2;                             // u32_t location;
    Attribute->binding  = 0;                             // u32_t binding;
    Attribute->format   = VK_FORMAT_R32G32B32A32_SFLOAT; // VkFormat format;
    Attribute->offset   = offsetof(vertex_point,Color);  // u32_t offset;

    Attribute = (InputsDescription.Attributes + 3);

    Attribute->location = 3;                             // u32_t location;
    Attribute->binding  = 0;                             // u32_t binding;
    Attribute->format   = VK_FORMAT_R32G32_SFLOAT; // VkFormat format;
    Attribute->offset   = offsetof(vertex_point,UV);  // u32_t offset;

    InputsDescription.Bindings[0] = Binding; // CONSTANTARRAY   Bindings;

    return InputsDescription;
}






VkViewport
VH_CreateDefaultViewport(VkExtent2D WindowExtent)
{
    VkViewport Viewport;

    Viewport.x        = 0;                                 // FLOAT x;
    Viewport.width    = (r32)WindowExtent.width;        // FLOAT width;
    
    // Invertex Axis Y as vulkan by default is top bottom
    Viewport.height   = (r32)WindowExtent.height*-1.0f; // FLOAT height;
    Viewport.y        = (r32)WindowExtent.height;       // FLOAT y;
    
    Viewport.minDepth = 0;                                 // FLOAT minDepth;
    Viewport.maxDepth = 1;                                 // FLOAT maxDepth;

    return Viewport;
}

VkPipeline
VH_PipelineBuilder(vulkan_pipeline * VulkanPipeline,VkDevice Device, VkRenderPass RenderPass, u32 Subpass = 0)
{
    VkPipelineViewportStateCreateInfo ViewportState = {};
    ViewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO; // VkStructureType sType;
    ViewportState.pNext         = 0;                         // Void * pNext;
    ViewportState.flags         = 0;                         // VkPipelineViewportStateCreateFlags flags;
    ViewportState.viewportCount = 1;                         // u32_t viewportCount;
    ViewportState.pViewports    = &VulkanPipeline->Viewport; // Typedef * pViewports;
    ViewportState.scissorCount  = 1;                         // u32_t scissorCount;
    ViewportState.pScissors     = &VulkanPipeline->Scissor;  // Typedef * pScissors;
    
    VkPipelineColorBlendStateCreateInfo ColorBlending = {};
    ColorBlending.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO; // VkStructureType sType;
    ColorBlending.pNext           = 0;                                     // Void * pNext;
    ColorBlending.flags           = 0;                                     // VkPipelineColorBlendStateCreateFlags flags;
    ColorBlending.logicOpEnable   = VK_FALSE;                              // VkBool32 logicOpEnable;
    ColorBlending.logicOp         = VK_LOGIC_OP_COPY;                      // VkLogicOp logicOp;
    ColorBlending.attachmentCount = VulkanPipeline->ColorBlendAttachmentCount;                                     // u32_t attachmentCount;
    ColorBlending.pAttachments    = &VulkanPipeline->ColorBlendAttachment[0]; // Typedef * pAttachments;
    //ColorBlending.blendConstants  = 0;// CONSTANTARRAY blendConstants;

    VkDynamicState DynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT, 
        VK_DYNAMIC_STATE_SCISSOR
    };

    // Dynamic vw/sci saves from having to resize pipelines every time window is resized
    // but you need to use vkCmdSetViewport, vkCmdSetScissor
    // you only call them for the first time or after a static pipeline
    VkPipelineDynamicStateCreateInfo DynamicStatesInfo = {};
    DynamicStatesInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicStatesInfo.flags = 0;
    DynamicStatesInfo.dynamicStateCount = ArrayCount(DynamicStates);
    DynamicStatesInfo.pDynamicStates =  DynamicStates;

    VkGraphicsPipelineCreateInfo PipelineInfo = {};
    PipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO; // VkStructureType sType;
    PipelineInfo.pNext               = 0;                                 // Void * pNext;
    PipelineInfo.flags               = 0;                                 // VkPipelineCreateFlags flags;
    PipelineInfo.stageCount          = VulkanPipeline->ShaderStagesCount; // u32_t stageCount;
    PipelineInfo.pStages             = VulkanPipeline->ShaderStages;      // Typedef * pStages;
    PipelineInfo.pVertexInputState   = &VulkanPipeline->VertexInputInfo;  // Typedef * pVertexInputState;
    PipelineInfo.pInputAssemblyState = &VulkanPipeline->InputAssembly;    // Typedef * pInputAssemblyState;
    PipelineInfo.pTessellationState  = 0;                                 // Typedef * pTessellationState;
    PipelineInfo.pViewportState      = &ViewportState;                    // Typedef * pViewportState;
    PipelineInfo.pRasterizationState = &VulkanPipeline->Rasterizer;       // Typedef * pRasterizationState;
    PipelineInfo.pMultisampleState   = &VulkanPipeline->Multisampling;    // Typedef * pMultisampleState;
    PipelineInfo.pDepthStencilState  = &VulkanPipeline->DepthStencil;     // Typedef * pDepthStencilState;
    PipelineInfo.pColorBlendState    = &ColorBlending;                    // Typedef * pColorBlendState;
    PipelineInfo.pDynamicState       = &DynamicStatesInfo;                                 // Typedef * pDynamicState;
    PipelineInfo.layout              = VulkanPipeline->PipelineLayout;    // VkPipelineLayout layout;
    PipelineInfo.renderPass          = RenderPass;                        // VkRenderPass renderPass;
    PipelineInfo.subpass             = Subpass;                                 // u32_t subpass;
    PipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;                    // VkPipeline basePipelineHandle;
    PipelineInfo.basePipelineIndex   = 0;                                 // i32_t basePipelineIndex;
    
    VkPipeline Pipeline;

    if (VK_FAILS(vkCreateGraphicsPipelines(Device,VK_NULL_HANDLE,1,&PipelineInfo, 0, &Pipeline)))
    {
        Log("Failed to create pipeline\n");
        return VK_NULL_HANDLE;
    }

    return Pipeline;
}

i32
VH_CreateCommandPool(VkDevice Device,
                        u32 QueueFamilyIndex,
                        VkCommandPoolCreateFlags CommandPoolCreateFlags,
                        VkCommandPool * CommandPool)
{
    VkCommandPoolCreateInfo CommandPoolCreateInfo;

    CommandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO; // VkStructureType sType;
    CommandPoolCreateInfo.pNext            = 0;                      // Void * pNext;
    CommandPoolCreateInfo.flags            = CommandPoolCreateFlags; // VkCommandPoolCreateFlags flags;
    CommandPoolCreateInfo.queueFamilyIndex = QueueFamilyIndex;       // u32_t queueFamilyIndex;

    VK_CHECK(vkCreateCommandPool(Device,&CommandPoolCreateInfo,0,CommandPool));

    return 0;
}

i32
VH_CreateCommandBuffers(VkDevice Device,VkCommandPool CommandPool,u32 CommandBufferCount,VkCommandBuffer * CommandBuffers)
{
    VkCommandBufferAllocateInfo CommandBufferAllocateInfo;

    CommandBufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; // VkStructureType   sType;
    CommandBufferAllocateInfo.pNext              = 0;                               // Void * pNext;
    CommandBufferAllocateInfo.commandPool        = CommandPool;                     // VkCommandPool commandPool;
    CommandBufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // VkCommandBufferLevel level;
    CommandBufferAllocateInfo.commandBufferCount = CommandBufferCount;              // u32_t commandBufferCount;

    VK_CHECK(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo , CommandBuffers));

    return 0;
}


VkPipelineLayoutCreateInfo
VH_CreatePipelineLayoutCreateInfo()
{

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo;

    PipelineLayoutCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // VkStructureType sType;
    PipelineLayoutCreateInfo.pNext                  = 0; // Void * pNext;
    PipelineLayoutCreateInfo.flags                  = 0; // VkPipelineLayoutCreateFlags flags;
    PipelineLayoutCreateInfo.setLayoutCount         = 0; // u32_t setLayoutCount;
    PipelineLayoutCreateInfo.pSetLayouts            = 0; // Typedef * pSetLayouts;
    PipelineLayoutCreateInfo.pushConstantRangeCount = 0; // u32_t pushConstantRangeCount;
    PipelineLayoutCreateInfo.pPushConstantRanges    = 0; // Typedef * pPushConstantRanges;

    return PipelineLayoutCreateInfo;
}

VkPipelineShaderStageCreateInfo
VH_CreateShaderStageInfo(VkShaderStageFlagBits Stage, VkShaderModule Module)
{
    VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo;
    PipelineShaderStageCreateInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; // VkStructureType   sType;
    PipelineShaderStageCreateInfo.pNext               = 0;      // Void * pNext;
    PipelineShaderStageCreateInfo.flags               = 0;      // VkPipelineShaderStageCreateFlags   flags;
    PipelineShaderStageCreateInfo.stage               = Stage;  // VkShaderStageFlagBits   stage;
    PipelineShaderStageCreateInfo.module              = Module; // VkShaderModule   module;
    PipelineShaderStageCreateInfo.pName               = "main"; // Char_S * pName;
    PipelineShaderStageCreateInfo.pSpecializationInfo = 0;      // Typedef * pSpecializationInfo;

    return PipelineShaderStageCreateInfo;
}

VkPipelineVertexInputStateCreateInfo
VH_CreateVertexInputStateInfo()
{
    VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo;
    PipelineVertexInputStateCreateInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO; // VkStructureType   sType;
    PipelineVertexInputStateCreateInfo.pNext                           = 0; // Void * pNext;
    PipelineVertexInputStateCreateInfo.flags                           = 0; // VkPipelineVertexInputStateCreateFlags flags;
    PipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount   = 0; // u32_t vertexBindingDescriptionCount;
    PipelineVertexInputStateCreateInfo.pVertexBindingDescriptions      = 0; // Typedef * pVertexBindingDescriptions;
    PipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0; // u32_t vertexAttributeDescriptionCount;
    PipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions    = 0; // Typedef * pVertexAttributeDescriptions;

    return PipelineVertexInputStateCreateInfo;
}

VkPipelineInputAssemblyStateCreateInfo
VH_CreatePipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology Topology)
{
    VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo;
    PipelineInputAssemblyStateCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO; // VkStructureType   sType;
    PipelineInputAssemblyStateCreateInfo.pNext                  = 0; // Void * pNext;
    PipelineInputAssemblyStateCreateInfo.flags                  = 0; // VkPipelineInputAssemblyStateCreateFlags   flags;
    PipelineInputAssemblyStateCreateInfo.topology               = Topology; // VkPrimitiveTopology   topology;
    PipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; // VkBool32   primitiveRestartEnable;

    return PipelineInputAssemblyStateCreateInfo;
}

VkPipelineRasterizationStateCreateInfo
VH_CreatePipelineRasterizationStateCreateInfo(VkPolygonMode PolygonMode)
{
    VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo;

    PipelineRasterizationStateCreateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO; // VkStructureType   sType;
    PipelineRasterizationStateCreateInfo.pNext                   = 0; // Void * pNext;
    PipelineRasterizationStateCreateInfo.flags                   = 0; // VkPipelineRasterizationStateCreateFlags   flags;
    PipelineRasterizationStateCreateInfo.depthClampEnable        = VK_FALSE; // VkBool32   depthClampEnable;
    PipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; // VkBool32   rasterizerDiscardEnable;
    PipelineRasterizationStateCreateInfo.polygonMode             = PolygonMode; // VkPolygonMode   polygonMode;
    PipelineRasterizationStateCreateInfo.cullMode                = VK_CULL_MODE_NONE; // VkCullModeFlags   cullMode;
    PipelineRasterizationStateCreateInfo.frontFace               = VK_FRONT_FACE_CLOCKWISE; // VkFrontFace   frontFace;
    PipelineRasterizationStateCreateInfo.depthBiasEnable         = VK_FALSE; // VkBool32   depthBiasEnable;
    PipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f; // FLOAT   depthBiasConstantFactor;
    PipelineRasterizationStateCreateInfo.depthBiasClamp          = 0.0f; // FLOAT   depthBiasClamp;
    PipelineRasterizationStateCreateInfo.depthBiasSlopeFactor    = 0.0f; // FLOAT   depthBiasSlopeFactor;
    PipelineRasterizationStateCreateInfo.lineWidth               = 1.0f; // FLOAT   lineWidth;

    return PipelineRasterizationStateCreateInfo;
}

VkPipelineMultisampleStateCreateInfo 
VH_CreatePipelineMultisampleStateCreateInfo()
{
    VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo;

    PipelineMultisampleStateCreateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; // VkStructureType   sType;
    PipelineMultisampleStateCreateInfo.pNext                 = 0; // Void * pNext;
    PipelineMultisampleStateCreateInfo.flags                 = 0; // VkPipelineMultisampleStateCreateFlags   flags;
    PipelineMultisampleStateCreateInfo.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT; // VkSampleCountFlagBits   rasterizationSamples;
    PipelineMultisampleStateCreateInfo.sampleShadingEnable   = VK_FALSE; // VkBool32   sampleShadingEnable;
    PipelineMultisampleStateCreateInfo.minSampleShading      = 1.0f; // FLOAT   minSampleShading;
    PipelineMultisampleStateCreateInfo.pSampleMask           = 0; // Typedef * pSampleMask;
    PipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // VkBool32   alphaToCoverageEnable;
    PipelineMultisampleStateCreateInfo.alphaToOneEnable      = VK_FALSE; // VkBool32   alphaToOneEnable;

    return PipelineMultisampleStateCreateInfo;
}

VkPipelineColorBlendAttachmentState
VH_CreatePipelineColorBlendAttachmentState()
{
    VkPipelineColorBlendAttachmentState PipelineColorBlendAttachmentState = {};

#if 0
    PipelineColorBlendAttachmentState.blendEnable         = VK_FALSE; // VkBool32   blendEnable;
#else
    PipelineColorBlendAttachmentState.blendEnable         = VK_TRUE; // VkBool32   blendEnable;
    PipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;           // VkBlendFactor srcColorBlendFactor;
    PipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // VkBlendFactor dstColorBlendFactor;
    PipelineColorBlendAttachmentState.colorBlendOp        = VK_BLEND_OP_ADD;                     // VkBlendOp colorBlendOp;
    PipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;                 // VkBlendFactor srcAlphaBlendFactor;
    PipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;                // VkBlendFactor dstAlphaBlendFactor;
    PipelineColorBlendAttachmentState.alphaBlendOp        = VK_BLEND_OP_ADD;                      // VkBlendOp alphaBlendOp;
#endif
    PipelineColorBlendAttachmentState.colorWriteMask      = 
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // VkColorComponentFlags   colorWriteMask;

    return PipelineColorBlendAttachmentState;
};

VkDeviceSize
VH_PaddedStorageBuffer(VkDeviceSize Size)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(GlobalVulkan.PrimaryGPU, &properties);
    VkDeviceSize Align = 
        (properties.limits.minStorageBufferOffsetAlignment - 1);
    
    VkDeviceSize AlignedSize = (Size + Align) & ~Align;

    return AlignedSize;
}

VkDeviceSize
VH_PaddedUniformBuffer(VkDeviceSize Size)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(GlobalVulkan.PrimaryGPU, &properties);
    VkDeviceSize Align = 
        (properties.limits.minUniformBufferOffsetAlignment - 1);

    VkDeviceSize AlignedSize = (Size + Align) & ~Align;

    return AlignedSize;
}

i32
CreateTransparencyPipeline(i32 VertexShaderIndex, i32 WeightFragmentShaderIndex,i32 FullscreenTriangleVertexShaderIndex, i32 CompositeFragmentShaderIndex)
{
    void * PipelineSubpass1 = 0;
    void * PipelineSubpass2 = 0;

    Assert((VertexShaderIndex         >= 0) && (VertexShaderIndex         < (i32)ArrayCount(GlobalVulkan.ShaderModules)));
    Assert((WeightFragmentShaderIndex >= 0) && (WeightFragmentShaderIndex < (i32)ArrayCount(GlobalVulkan.ShaderModules)));

    Assert((FullscreenTriangleVertexShaderIndex >= 0) && (FullscreenTriangleVertexShaderIndex < (i32)ArrayCount(GlobalVulkan.ShaderModules)));
    Assert((CompositeFragmentShaderIndex        >= 0) && (CompositeFragmentShaderIndex        < (i32)ArrayCount(GlobalVulkan.ShaderModules)));

    vulkan_pipeline VulkanPipelines[2] = {};
    vulkan_pipeline * VulkanPipelineWeight    = VulkanPipelines + 0;
    vulkan_pipeline * VulkanPipelineComposite = VulkanPipelines + 1;

    /* SUBPASS 1 */
    {
        VkShaderModule VertexShader                        = GlobalVulkan.ShaderModules[VertexShaderIndex];
        VkShaderModule FragmentShader                      = GlobalVulkan.ShaderModules[WeightFragmentShaderIndex];
        vertex_inputs_description VertexInputDesc          = RenderGetVertexInputsDescription();
        VkPipelineDepthStencilStateCreateInfo DepthStencil = {};
        VkPipelineColorBlendAttachmentState Subpass1ColorBlendAttach[2] = {};
        VkPipelineColorBlendAttachmentState * Subpass1ColorBlendAttachWeight = Subpass1ColorBlendAttach + 0;
        VkPipelineColorBlendAttachmentState * Subpass1ColorBlendAttachReveal = Subpass1ColorBlendAttach + 1;

        VulkanPipelineWeight->ShaderStagesCount = 2; // u32 ShaderStagesCount;
        VulkanPipelineWeight->ShaderStages[0]   = VH_CreateShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT,VertexShader);
        VulkanPipelineWeight->ShaderStages[1]   = VH_CreateShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT,FragmentShader);

        VulkanPipelineWeight->VertexInputInfo      = VH_CreateVertexInputStateInfo(); // VkPipelineVertexInputStateCreateInfo   VertexInputInfo;

        VulkanPipelineWeight->VertexInputInfo.vertexBindingDescriptionCount   = 1;                              // u32_t vertexBindingDescriptionCount;
        VulkanPipelineWeight->VertexInputInfo.pVertexBindingDescriptions      = &VertexInputDesc.Bindings[0];   // Typedef * pVertexBindingDescriptions;
        VulkanPipelineWeight->VertexInputInfo.vertexAttributeDescriptionCount = ArrayCount(VertexInputDesc.Attributes);                              // u32_t vertexAttributeDescriptionCount;
        VulkanPipelineWeight->VertexInputInfo.pVertexAttributeDescriptions    = &VertexInputDesc.Attributes[0]; // Typedef * pVertexAttributeDescriptions;

        DepthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO; // VkStructureType sType;
        DepthStencil.depthTestEnable       = VK_TRUE;                                                    // VkBool32 depthTestEnable;
        DepthStencil.depthWriteEnable      = VK_FALSE;                                                   // VkBool32 depthWriteEnable;
        DepthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;                                // VkCompareOp depthCompareOp;
        DepthStencil.depthBoundsTestEnable = VK_FALSE;                                                   // VkBool32 depthBoundsTestEnable;
        DepthStencil.stencilTestEnable     = VK_FALSE;                                                   // VkBool32 stencilTestEnable;
        DepthStencil.minDepthBounds        = 0.0f;                                                       // FLOAT minDepthBounds;
        DepthStencil.maxDepthBounds        = 1.0f;                                                       // FLOAT maxDepthBounds;

        Subpass1ColorBlendAttachWeight->blendEnable         = VK_TRUE; // blendEnable   blendEnable
        Subpass1ColorBlendAttachWeight->srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // srcColorBlendFactor   srcColorBlendFactor
        Subpass1ColorBlendAttachWeight->dstColorBlendFactor = VK_BLEND_FACTOR_ONE; // dstColorBlendFactor   dstColorBlendFactor
        Subpass1ColorBlendAttachWeight->colorBlendOp        = VK_BLEND_OP_ADD; // colorBlendOp   colorBlendOp
        Subpass1ColorBlendAttachWeight->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // srcAlphaBlendFactor   srcAlphaBlendFactor
        Subpass1ColorBlendAttachWeight->dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // dstAlphaBlendFactor   dstAlphaBlendFactor
        Subpass1ColorBlendAttachWeight->alphaBlendOp        = VK_BLEND_OP_ADD; // alphaBlendOp   alphaBlendOp
        Subpass1ColorBlendAttachWeight->colorWriteMask      = 
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // VkColorComponentFlags   colorWriteMask;

        Subpass1ColorBlendAttachReveal->blendEnable         = VK_TRUE; // blendEnable   blendEnable
        Subpass1ColorBlendAttachReveal->srcColorBlendFactor = VK_BLEND_FACTOR_ZERO; // srcColorBlendFactor   srcColorBlendFactor
        Subpass1ColorBlendAttachReveal->dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR; // dstColorBlendFactor   dstColorBlendFactor
        Subpass1ColorBlendAttachReveal->colorBlendOp        = VK_BLEND_OP_ADD; // colorBlendOp   colorBlendOp
        Subpass1ColorBlendAttachReveal->srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // srcAlphaBlendFactor   srcAlphaBlendFactor
        Subpass1ColorBlendAttachReveal->dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // dstAlphaBlendFactor   dstAlphaBlendFactor
        Subpass1ColorBlendAttachReveal->alphaBlendOp        = VK_BLEND_OP_ADD; // alphaBlendOp   alphaBlendOp
        Subpass1ColorBlendAttachReveal->colorWriteMask      = 
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // VkColorComponentFlags   colorWriteMask;

        VulkanPipelineWeight->DepthStencil         = DepthStencil;
        VulkanPipelineWeight->InputAssembly        = VH_CreatePipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST); // VkPipelineInputAssemblyStateCreateInfo InputAssembly;
        VulkanPipelineWeight->Viewport             = VH_CreateDefaultViewport(GlobalVulkan.WindowExtension);                             // VkViewport Viewport;
        VulkanPipelineWeight->Scissor.offset       = {0,0};
        VulkanPipelineWeight->Scissor.extent       = GlobalVulkan.WindowExtension;
        VulkanPipelineWeight->Rasterizer           = VH_CreatePipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);                // VkPipelineRasterizationStateCreateInfo Rasterizer;
        VulkanPipelineWeight->ColorBlendAttachment[0] = *Subpass1ColorBlendAttachWeight;
        VulkanPipelineWeight->ColorBlendAttachment[1] = *Subpass1ColorBlendAttachReveal;
        VulkanPipelineWeight->ColorBlendAttachmentCount = 2;
        VulkanPipelineWeight->Multisampling        = VH_CreatePipelineMultisampleStateCreateInfo();                                      // VkPipelineMultisampleStateCreateInfo Multisampling;
        VulkanPipelineWeight->PipelineLayout       = GlobalVulkan.PipelineLayout[0];// VkPipelineLayout PipelineLayout;

        VkPipeline Pipeline = 
            VH_PipelineBuilder(VulkanPipelineWeight, GlobalVulkan.PrimaryDevice, GlobalVulkan.RenderPassTransparency);

        if (VK_VALID_HANDLE(Pipeline))
        {
            Assert(GlobalVulkan.PipelinesCount < ArrayCount(GlobalVulkan.Pipelines));

            i32 IndexPipeline = GlobalVulkan.PipelinesCount++;

            GlobalVulkan.PipelinesDefinition[IndexPipeline] = *VulkanPipelineWeight;
            GlobalVulkan.Pipelines[IndexPipeline]= Pipeline;

            VULKAN_TREE_APPEND(vkDestroyPipeline, GlobalVulkan.PipelinesLabel, GlobalVulkan.Pipelines[IndexPipeline]);
        }
    }
    
    /* SUBPASS 2 */
    {
        VkShaderModule VertexShader = GlobalVulkan.ShaderModules[FullscreenTriangleVertexShaderIndex];
        VkShaderModule FragmentShader = GlobalVulkan.ShaderModules[CompositeFragmentShaderIndex];
        VkPipelineDepthStencilStateCreateInfo DepthStencil = {};
        VkPipelineColorBlendAttachmentState Subpass2ColorBlendAttach;

        VulkanPipelineComposite->ShaderStagesCount = 2; // u32 ShaderStagesCount;
        VulkanPipelineComposite->ShaderStages[0]   = VH_CreateShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT,VertexShader);
        VulkanPipelineComposite->ShaderStages[1]   = VH_CreateShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT,FragmentShader);

        VkPipelineVertexInputStateCreateInfo NoVertexInputInfo = {};
        NoVertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO; // sType   sType
        NoVertexInputInfo.vertexBindingDescriptionCount   = 0; // vertexBindingDescriptionCount   vertexBindingDescriptionCount
        NoVertexInputInfo.vertexAttributeDescriptionCount = 0; // vertexAttributeDescriptionCount   vertexAttributeDescriptionCount

        VulkanPipelineComposite->VertexInputInfo      = NoVertexInputInfo; // VertexInputInfo   VertexInputInfo

        DepthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO; // VkStructureType sType;
        DepthStencil.depthTestEnable       = VK_TRUE;                                                    // VkBool32 depthTestEnable;
        DepthStencil.depthWriteEnable      = VK_FALSE;                                                   // VkBool32 depthWriteEnable;
        DepthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;                                // VkCompareOp depthCompareOp;
        DepthStencil.depthBoundsTestEnable = VK_FALSE;                                                   // VkBool32 depthBoundsTestEnable;
        DepthStencil.stencilTestEnable     = VK_FALSE;                                                   // VkBool32 stencilTestEnable;
        DepthStencil.minDepthBounds        = 0.0f;                                                       // FLOAT minDepthBounds;
        DepthStencil.maxDepthBounds        = 1.0f;                                                       // FLOAT maxDepthBounds;

        Subpass2ColorBlendAttach.blendEnable         = VK_TRUE; // blendEnable   blendEnable
        Subpass2ColorBlendAttach.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // srcColorBlendFactor   srcColorBlendFactor
        Subpass2ColorBlendAttach.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // dstColorBlendFactor   dstColorBlendFactor
        Subpass2ColorBlendAttach.colorBlendOp        = VK_BLEND_OP_ADD; // colorBlendOp   colorBlendOp
        Subpass2ColorBlendAttach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // srcAlphaBlendFactor   srcAlphaBlendFactor
        Subpass2ColorBlendAttach.dstAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // dstAlphaBlendFactor   dstAlphaBlendFactor
        Subpass2ColorBlendAttach.alphaBlendOp        = VK_BLEND_OP_ADD; // alphaBlendOp   alphaBlendOp
        Subpass2ColorBlendAttach.colorWriteMask      = 
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // VkColorComponentFlags   colorWriteMask;

        VulkanPipelineComposite->DepthStencil         = DepthStencil;
        VulkanPipelineComposite->InputAssembly        = VH_CreatePipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST); // VkPipelineInputAssemblyStateCreateInfo InputAssembly;
        VulkanPipelineComposite->Viewport             = VH_CreateDefaultViewport(GlobalVulkan.WindowExtension);                             // VkViewport Viewport;
        VulkanPipelineComposite->Scissor.offset       = {0,0};
        VulkanPipelineComposite->Scissor.extent       = GlobalVulkan.WindowExtension;
        VulkanPipelineComposite->Rasterizer           = VH_CreatePipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);                // VkPipelineRasterizationStateCreateInfo Rasterizer;
        VulkanPipelineComposite->ColorBlendAttachment[0] = Subpass2ColorBlendAttach;
        VulkanPipelineComposite->ColorBlendAttachmentCount = 1;
        VulkanPipelineComposite->Multisampling        = VH_CreatePipelineMultisampleStateCreateInfo();                                      // VkPipelineMultisampleStateCreateInfo Multisampling;
        VulkanPipelineComposite->PipelineLayout       = GlobalVulkan.PipelineLayout[0];                                                     // VkPipelineLayout PipelineLayout;

        VkPipeline Pipeline = 
            VH_PipelineBuilder(VulkanPipelineComposite, GlobalVulkan.PrimaryDevice, GlobalVulkan.RenderPassTransparency, 1);

        if (VK_VALID_HANDLE(Pipeline))
        {
            Assert(GlobalVulkan.PipelinesCount < ArrayCount(GlobalVulkan.Pipelines));

            i32 IndexPipeline = GlobalVulkan.PipelinesCount++;

            GlobalVulkan.PipelinesDefinition[IndexPipeline] = *VulkanPipelineComposite;
            GlobalVulkan.Pipelines[IndexPipeline]= Pipeline;


            VULKAN_TREE_APPEND(vkDestroyPipeline, GlobalVulkan.PipelinesLabel, GlobalVulkan.Pipelines[IndexPipeline]);
        }
    }

    return 0;
}

i32
CreatePipeline(i32 VertexShaderIndex, i32 FragmentShaderIndex, polygon_mode PolygonMode)
{
    Assert((VertexShaderIndex >= 0) && ((i32)ArrayCount(GlobalVulkan.ShaderModules) > VertexShaderIndex));
    Assert((FragmentShaderIndex >= 0) && ((i32)ArrayCount(GlobalVulkan.ShaderModules) > FragmentShaderIndex));

    VkShaderModule VertexShader = GlobalVulkan.ShaderModules[VertexShaderIndex];
    VkShaderModule FragmentShader = GlobalVulkan.ShaderModules[FragmentShaderIndex];

    vulkan_pipeline VulkanPipeline;

    VulkanPipeline.ShaderStagesCount = 2; // u32 ShaderStagesCount;
    VulkanPipeline.ShaderStages[0]   = VH_CreateShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT,VertexShader);
    VulkanPipeline.ShaderStages[1]   = VH_CreateShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT,FragmentShader);

    VulkanPipeline.VertexInputInfo      = VH_CreateVertexInputStateInfo(); // VkPipelineVertexInputStateCreateInfo   VertexInputInfo;

    vertex_inputs_description VertexInputDesc = RenderGetVertexInputsDescription();
    VulkanPipeline.VertexInputInfo.vertexBindingDescriptionCount   = 1;                              // u32_t vertexBindingDescriptionCount;
    VulkanPipeline.VertexInputInfo.pVertexBindingDescriptions      = &VertexInputDesc.Bindings[0];   // Typedef * pVertexBindingDescriptions;
    VulkanPipeline.VertexInputInfo.vertexAttributeDescriptionCount = ArrayCount(VertexInputDesc.Attributes);                              // u32_t vertexAttributeDescriptionCount;
    VulkanPipeline.VertexInputInfo.pVertexAttributeDescriptions    = &VertexInputDesc.Attributes[0]; // Typedef * pVertexAttributeDescriptions;


    VkPipelineDepthStencilStateCreateInfo DepthStencil = {};

    DepthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO; // VkStructureType   sType;
    DepthStencil.pNext                 = 0;                           // Void * pNext;
    DepthStencil.flags                 = 0;                           // VkPipelineDepthStencilStateCreateFlags flags;
    DepthStencil.depthTestEnable       = VK_TRUE;                     // VkBool32 depthTestEnable;
    DepthStencil.depthWriteEnable      = VK_TRUE;                     // VkBool32 depthWriteEnable;
    DepthStencil.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL; // VkCompareOp depthCompareOp;
    //DepthStencil.depthCompareOp        = VK_COMPARE_OP_LESS; // VkCompareOp depthCompareOp;
    DepthStencil.depthBoundsTestEnable = VK_FALSE;                    // VkBool32 depthBoundsTestEnable;
    DepthStencil.stencilTestEnable     = VK_FALSE;                    // VkBool32 stencilTestEnable;
    //DepthStencil.front               = ; // VkStencilOpState front;
    //DepthStencil.back                = ; // VkStencilOpState back;
    DepthStencil.minDepthBounds        = 0.0f;                        // FLOAT minDepthBounds;
    DepthStencil.maxDepthBounds        = 1.0f;                        // FLOAT maxDepthBounds;
    
    VulkanPipeline.DepthStencil = DepthStencil;

#if 1
    VulkanPipeline.InputAssembly        = 
        VH_CreatePipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST); // VkPipelineInputAssemblyStateCreateInfo   InputAssembly;
#else
    //VulkanPipeline.InputAssembly        = VH_CreatePipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_LINE_LIST); // VkPipelineInputAssemblyStateCreateInfo   InputAssembly;
    VulkanPipeline.InputAssembly        = VH_CreatePipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY); // VkPipelineInputAssemblyStateCreateInfo   InputAssembly;
#endif

    VulkanPipeline.Viewport             = VH_CreateDefaultViewport(GlobalVulkan.WindowExtension); // VkViewport   Viewport;

    VulkanPipeline.Scissor.offset = {0,0};
    VulkanPipeline.Scissor.extent = GlobalVulkan.WindowExtension;

    VkPolygonMode vkPolygonMode = VK_POLYGON_MODE_FILL;

    switch (PolygonMode)
    {
        case polygon_mode_fill: { } break;
        case polygon_mode_line:
        {
            vkPolygonMode = VK_POLYGON_MODE_LINE;
        } break;
        default:
        {
            Assert(0); // Not implemented
        };
    }

    VulkanPipeline.Rasterizer           = 
        VH_CreatePipelineRasterizationStateCreateInfo(vkPolygonMode); // VkPipelineRasterizationStateCreateInfo   Rasterizer;

    VulkanPipeline.ColorBlendAttachment[0] = VH_CreatePipelineColorBlendAttachmentState();  // VkPipelineColorBlendAttachmentState ColorBlendAttachment;
    VulkanPipeline.ColorBlendAttachmentCount = 1;

    VulkanPipeline.Multisampling        = VH_CreatePipelineMultisampleStateCreateInfo(); // VkPipelineMultisampleStateCreateInfo Multisampling;
    VulkanPipeline.PipelineLayout       = GlobalVulkan.PipelineLayout[0];                      // VkPipelineLayout PipelineLayout;

    VkPipeline Pipeline = VH_PipelineBuilder(&VulkanPipeline, GlobalVulkan.PrimaryDevice, GlobalVulkan.RenderPass);

    if (VK_VALID_HANDLE(Pipeline))
    {
        Assert(GlobalVulkan.PipelinesCount < ArrayCount(GlobalVulkan.Pipelines));

        i32 IndexPipeline = GlobalVulkan.PipelinesCount++;

        GlobalVulkan.PipelinesDefinition[IndexPipeline] = VulkanPipeline;
        GlobalVulkan.Pipelines[IndexPipeline]= Pipeline;

#if 0
        Result.Pipeline = IndexPipeline;
        Result.Success = true;
        Result.PipelineLayout = 0; // only 1 for now, hardcoded as the first 
#endif

        VULKAN_TREE_APPEND(vkDestroyPipeline, GlobalVulkan.PipelinesLabel, GlobalVulkan.Pipelines[IndexPipeline]);
    }

    //return Result;
    return 0;
}

struct vulkan_device_extensions
{
    const char * DeviceExtensions[1];
    u32 Count;
};

vulkan_device_extensions
GetRequiredDeviceExtensions()
{
    vulkan_device_extensions Ext = {};
    Ext.DeviceExtensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME; // CONSTANTARRAY   DeviceExtensions;
    ++Ext.Count;
    return Ext;
}

struct surface_capabilities
{
    VkExtent2D                      SwapChainExtent;
    u32                          ImageCount;
    VkImageUsageFlags               ImageUsageFlags;
    b32                          ValidSurface;
    VkSurfaceTransformFlagBitsKHR   SurfaceTransforms;
    b32                          Minimized;
};

i32
VulkanMapArena(gpu_arena * Arena)
{
    if (IS_NULL(Arena->WriteToAddr))
    {
        VkMemoryMapFlags Flags = 0; // RESERVED FUTURE USE

        VkDeviceMemory DeviceMemory = GetDeviceMemory(Arena->MemoryIndexType);

        if (VK_FAILS(vkMapMemory(Arena->Device, DeviceMemory, 
                                 Arena->DeviceBindingOffsetBegin, 
                                 Arena->MaxSize, Flags , &Arena->WriteToAddr)))
        {
            Logn("Failed to initiate mapping on Objects buffer");
            return 1;
        }
    }

    return 0;
}
void
VulkanUnmapArena(gpu_arena * Arena)
{
    Assert(IS_NOT_NULL(Arena->WriteToAddr));

    VkDeviceMemory DeviceMemory = GetDeviceMemory(Arena->MemoryIndexType);

    vkUnmapMemory(Arena->Device,DeviceMemory);

    Arena->WriteToAddr = 0;
}

inline frame_data *
GetCurrentFrame()
{
    frame_data * FrameData = GlobalVulkan.FrameData + (GlobalVulkan._CurrentFrameData % FRAME_OVERLAP);
    
    return FrameData;
}



gpu_memory_mapping_result
BeginObjectMapping(u32 Units)
{
    gpu_memory_mapping_result Result = {};

    gpu_arena * Arena = GetCurrentFrame()->ObjectsArena;
    u64 BeginOffsetUnits = Arena->CurrentSize / sizeof(GPUObjectData);
    u32 TotalUnitsSize = Units * sizeof(GPUObjectData);
    Assert((Arena->CurrentSize + TotalUnitsSize) <= Arena->MaxSize);

    i32 Error = VulkanMapArena(Arena);

    if (Error == 0)
    {
        Result.BeginAddress = (GPUObjectData *)Arena->WriteToAddr + BeginOffsetUnits;
        Result.Instance = BeginOffsetUnits;
        Result.Success = true;
        Arena->CurrentSize += TotalUnitsSize;
    }

    return Result;
}
void
EndObjectsArena()
{
    gpu_arena * Arena = GetCurrentFrame()->ObjectsArena;
    VulkanUnmapArena(Arena);
}

i32
VulkanGetSurfaceCapabilities(VkPhysicalDevice PhysicalDevice,VkSurfaceKHR Surface,
                             u32 Width, u32 Height,
                             surface_capabilities * GPUSurfaceCapabilities)
{
    VkSurfaceCapabilitiesKHR SurfaceCapabilities;

    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &SurfaceCapabilities));

    GPUSurfaceCapabilities->ImageCount = SurfaceCapabilities.minImageCount + 1;

    if ( ( SurfaceCapabilities.maxImageCount > 0 ) && 
         ( GPUSurfaceCapabilities->ImageCount > SurfaceCapabilities.maxImageCount ) 
        )
    {
        GPUSurfaceCapabilities->ImageCount = SurfaceCapabilities.maxImageCount;
    }

    if ( SurfaceCapabilities.currentExtent.width == 0)
    {
        VkExtent2D e = { Width, Height };

        e.width = (e.width < SurfaceCapabilities.minImageExtent.width) ? SurfaceCapabilities.minImageExtent.width : e.width;
        e.width = (e.width > SurfaceCapabilities.maxImageExtent.width) ? SurfaceCapabilities.maxImageExtent.width : e.width;

        e.height = (e.height < SurfaceCapabilities.minImageExtent.height) ? SurfaceCapabilities.minImageExtent.height : e.height;
        e.height = (e.height > SurfaceCapabilities.maxImageExtent.height) ? SurfaceCapabilities.maxImageExtent.height : e.height;
        
        GPUSurfaceCapabilities->SwapChainExtent = e;
    }
    else
    {
        GPUSurfaceCapabilities->SwapChainExtent = SurfaceCapabilities.currentExtent;
    }

    GPUSurfaceCapabilities->Minimized = (SurfaceCapabilities.currentExtent.width == 0);

    if ( ( SurfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT ) )
    {
        GPUSurfaceCapabilities->ImageUsageFlags = 
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    else
    {
        Log("Image transfer destination bit not supported\n");
        return 1;
    }

    if ( ( SurfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ) )
    {
        GPUSurfaceCapabilities->SurfaceTransforms =  VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        GPUSurfaceCapabilities->SurfaceTransforms =  SurfaceCapabilities.currentTransform;
    }

    return 0;
}

i32
GetVulkanSurfaceFormat(VkPhysicalDevice PhysicalDevice,VkSurfaceKHR Surface,VkSurfaceFormatKHR * Format)
{
    *Format = { VK_FORMAT_UNDEFINED, VK_COLORSPACE_SRGB_NONLINEAR_KHR };

    u32 FormatsCount = 0;

    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatsCount,0));

    VkSurfaceFormatKHR SurfaceFormats[20];
    Assert(ArrayCount(SurfaceFormats) >= FormatsCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatsCount, &SurfaceFormats[0]));

    // If only 1 format available use this default
    if (    ( FormatsCount == 1 ) &&
            ( SurfaceFormats[0].format == VK_FORMAT_UNDEFINED )
       )
    {
        *Format = { VK_FORMAT_R8G8B8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
    } 
    else
    {
        for (u32 SurfaceFormatsIndex = 0;
                    SurfaceFormatsIndex < ArrayCount(SurfaceFormats);
                    ++SurfaceFormatsIndex)
        {
            if (SurfaceFormats[SurfaceFormatsIndex].format == VK_FORMAT_R8G8B8_UNORM)
            {
                *Format = SurfaceFormats[SurfaceFormatsIndex];
                break;
            }
        }
    }

    // Our preferred way was std RGB but if not found just choose whatever
    if (Format->format == VK_FORMAT_UNDEFINED)
    {
        *Format = SurfaceFormats[0];
    }

    return 0;
}

i32
GetVulkanSurfacePresentMode(VkPhysicalDevice PhysicalDevice,VkSurfaceKHR Surface,VkPresentModeKHR * PresentMode)
{
    *PresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

    u32 PresentModesCount;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModesCount,0));

    VkPresentModeKHR SurfacePresentModes[20];
    Assert(ArrayCount(SurfacePresentModes) >= PresentModesCount);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModesCount, &SurfacePresentModes[0]));

    b32 PresentModeFound = false;
    for (u32 PresentModeIndex = 0;
            PresentModeIndex < ArrayCount(SurfacePresentModes);
            ++PresentModeIndex)
    {
        if ( SurfacePresentModes[PresentModeIndex] == VK_PRESENT_MODE_MAILBOX_KHR )
        {
            *PresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            PresentModeFound = true;
            break;
        }
    }
    if ( ( !PresentModeFound ) )
    {

        for (u32 PresentModeIndex = 0;
                PresentModeIndex < ArrayCount(SurfacePresentModes);
                ++PresentModeIndex)
        {
            if ( SurfacePresentModes[PresentModeIndex] == VK_PRESENT_MODE_FIFO_KHR )
            {
                *PresentMode = VK_PRESENT_MODE_FIFO_KHR;
                PresentModeFound = true;
                break;
            }
        }
    }

    if ( ( !PresentModeFound ) )
    {
        Log("Present mode FIFO/MAILBOX is not supported\n");
        return 1;
    }

    return 0;
}
void
VulkanWaitForDevices()
{
    if (VK_VALID_HANDLE(GlobalVulkan.PrimaryDevice))
    {
        vkDeviceWaitIdle(GlobalVulkan.PrimaryDevice); 
    }

    if (VK_VALID_HANDLE(GlobalVulkan.SecondaryDevice))
    {
        vkDeviceWaitIdle(GlobalVulkan.SecondaryDevice); 
    }
}

i32
VulkanCreateSwapChain(i32 Width, i32 Height)
{

#if 1
    b32 KeepSwapchainAlive = true;
    HierarchyTreeDropBranch(GlobalVulkan.HTree, GlobalVulkan.Swapchain, KeepSwapchainAlive);
#endif

    surface_capabilities Capabilities = {};
    if (VulkanGetSurfaceCapabilities(GlobalVulkan.PrimaryGPU,GlobalVulkan.Surface,Width,Height,&Capabilities))
    {
        Log("Couldn't read surface capabilities\n");
        return 1;
    }

    GlobalWindowIsMinimized =  Capabilities.Minimized;

    if (GlobalWindowIsMinimized) return 0;

    VkSurfaceFormatKHR Format = {};
    if (GetVulkanSurfaceFormat(GlobalVulkan.PrimaryGPU,GlobalVulkan.Surface,&Format))
    {
        Log("Couldn't read surface format\n");
        return 1;
    }

    VkPresentModeKHR PresentMode;
    if ( GetVulkanSurfacePresentMode(GlobalVulkan.PrimaryGPU,GlobalVulkan.Surface,&PresentMode) )
    {
        Log("Couldn't read surface present mode\n");    
        return 1;
    }

    VkSwapchainKHR OldSwapChain = GlobalVulkan.Swapchain;

    VkSwapchainCreateInfoKHR SwapChainCreateInfo;
    SwapChainCreateInfo.sType                  = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR; // VkStructureType   sType;
    SwapChainCreateInfo.pNext                  = 0;                                           // Void * pNext;
    SwapChainCreateInfo.flags                  = 0;                                           // VkSwapchainCreateFlagsKHR   flags;
    SwapChainCreateInfo.surface                = GlobalVulkan.Surface;                       // VkSurfaceKHR   surface;
    SwapChainCreateInfo.minImageCount          = Capabilities.ImageCount;                     // u32_t   minImageCount;
    SwapChainCreateInfo.imageFormat            = Format.format;                               // VkFormat   imageFormat;
    SwapChainCreateInfo.imageColorSpace        = Format.colorSpace;                           // VkColorSpaceKHR   imageColorSpace;
    SwapChainCreateInfo.imageExtent            = Capabilities.SwapChainExtent;                // VkExtent2D   imageExtent;
    SwapChainCreateInfo.imageArrayLayers       = 1;                                           // u32_t   imageArrayLayers;
    SwapChainCreateInfo.imageUsage             = Capabilities.ImageUsageFlags;                // VkImageUsageFlags   imageUsage;
    SwapChainCreateInfo.imageSharingMode       = VK_SHARING_MODE_EXCLUSIVE;                   // VkSharingMode   imageSharingMode;
    SwapChainCreateInfo.queueFamilyIndexCount  = 0;                                           // u32_t   queueFamilyIndexCount;
    SwapChainCreateInfo.pQueueFamilyIndices    = 0;                                           // Typedef * pQueueFamilyIndices;
    SwapChainCreateInfo.preTransform           = Capabilities.SurfaceTransforms;              // VkSurfaceTransformFlagBitsKHR   preTransform;
    SwapChainCreateInfo.compositeAlpha         = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;           // VkCompositeAlphaFlagBitsKHR   compositeAlpha;
    SwapChainCreateInfo.presentMode            = PresentMode;                                 // VkPresentModeKHR   presentMode;
    SwapChainCreateInfo.clipped                = VK_TRUE;                                     // VkBool32   clipped;
    SwapChainCreateInfo.oldSwapchain           = OldSwapChain;                                // VkSwapchainKHR   oldSwapchain;

    VK_CHECK(vkCreateSwapchainKHR(GlobalVulkan.PrimaryDevice, &SwapChainCreateInfo, 0, &GlobalVulkan.Swapchain));

    GlobalVulkan.SwapchainImageFormat = Format.format;
    GlobalVulkan.WindowExtension      = Capabilities.SwapChainExtent;
    GlobalVulkan.SwapchainImageCount  = Capabilities.ImageCount;

    // Must always be destroyed AFTER creation of new swapchain as create info struct points to old
    if ( VK_VALID_HANDLE(OldSwapChain) )
    {
        tree_node * SwapchainNode = HierarchyTreeFind(GlobalVulkan.HTree, OldSwapChain);
        Assert(SwapchainNode);
        ((vulkan_node *)SwapchainNode->Data)->ID = GlobalVulkan.Swapchain;
        vkDestroySwapchainKHR(GlobalVulkan.PrimaryDevice, OldSwapChain, 0);
    }
    else
    {
        VULKAN_TREE_APPEND(vkDestroySwapchainKHR, GlobalVulkan.Surface, GlobalVulkan.Swapchain); 
    }

    u32 SwapchainImageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR( GlobalVulkan.PrimaryDevice, GlobalVulkan.Swapchain, &SwapchainImageCount, 0));
    VK_CHECK(vkGetSwapchainImagesKHR( GlobalVulkan.PrimaryDevice, GlobalVulkan.Swapchain, &SwapchainImageCount, &GlobalVulkan.SwapchainImages[0]));

    VkComponentMapping RGBAComponents = {
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY
    };

    VkImageSubresourceRange ImageSubresourceRange;
    ImageSubresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT; // VkImageAspectFlags   aspectMask;
    ImageSubresourceRange.baseMipLevel   = 0; // u32_t   baseMipLevel;
    ImageSubresourceRange.levelCount     = 1; // u32_t   levelCount;
    ImageSubresourceRange.baseArrayLayer = 0; // u32_t   baseArrayLayer;
    ImageSubresourceRange.layerCount     = 1; // u32_t   layerCount;

    for (u32 ImageIndex = 0;
                ImageIndex < SwapchainImageCount;
                ++ImageIndex)
    {
        VkImageViewCreateInfo ImageViewCreateInfo;
        ImageViewCreateInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; // VkStructureType   sType;
        ImageViewCreateInfo.pNext            = 0;                                        // Void * pNext;
        ImageViewCreateInfo.flags            = 0;                                        // VkImageViewCreateFlags   flags;
        ImageViewCreateInfo.image            = GlobalVulkan.SwapchainImages[ImageIndex]; // VkImage   image;
        ImageViewCreateInfo.viewType         = VK_IMAGE_VIEW_TYPE_2D;                    // VkImageViewType   viewType;
        ImageViewCreateInfo.format           = Format.format;                            // VkFormat   format;
        ImageViewCreateInfo.components       = RGBAComponents;                           // VkComponentMapping   components;
        ImageViewCreateInfo.subresourceRange = ImageSubresourceRange;                    // VkImageSubresourceRange   subresourceRange;

        VK_CHECK(vkCreateImageView( GlobalVulkan.PrimaryDevice, &ImageViewCreateInfo, 0, &GlobalVulkan.SwapchainImageViews[ImageIndex] ));
        VULKAN_TREE_APPEND(vkDestroyImageView, GlobalVulkan.Swapchain,GlobalVulkan.SwapchainImageViews[ImageIndex]);
    }

    return 0;
}

i32
VulkanCreateTransparentRenderPass()
{
    VkAttachmentDescription WeightedColorAttachment = {};
    WeightedColorAttachment.format         = GlobalVulkan.WeightedColorImage->Format; // format   format
    WeightedColorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT; // samples   samples
    WeightedColorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR; // loadOp   loadOp
    WeightedColorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE; // storeOp   storeOp
    WeightedColorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // stencilLoadOp   stencilLoadOp
    WeightedColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // stencilStoreOp   stencilStoreOp
    WeightedColorAttachment.initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // initialLayout   initialLayout
    WeightedColorAttachment.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // finalLayout   finalLayout

    VkAttachmentDescription WeightedRevealAttachment = WeightedColorAttachment;
    WeightedRevealAttachment.format = GlobalVulkan.WeightedRevealImage->Format;

    VkAttachmentDescription ColorAttachment = WeightedColorAttachment;
    ColorAttachment.format         = GlobalVulkan.SwapchainImageFormat; // VkFormat   format;
    ColorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;       // VkAttachmentLoadOp   loadOp;
    ColorAttachment.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;   // VkImageLayout   finalLayout;

    VkAttachmentDescription DepthAttachment = ColorAttachment;
    DepthAttachment.format         = GlobalVulkan.PrimaryDepthBuffer->Format;                  // VkFormat format;
    DepthAttachment.initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;                        // VkImageLayout initialLayout;
    DepthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // VkImageLayout finalLayout;

    VkAttachmentDescription Attachments[] = {
        WeightedColorAttachment,
        WeightedRevealAttachment,
        ColorAttachment,
        DepthAttachment
    };

    /* BEGIN SUBPASS 1*/
    VkSubpassDescription Subpasses[2] = {};
    VkSubpassDescription * Subpass1 = Subpasses + 0;
    VkSubpassDescription * Subpass2 = Subpasses + 1;

    VkAttachmentReference Subpass1ColorAttachment[2] = {};
    VkAttachmentReference DepthAttachmentReference = {};

    Subpass1ColorAttachment[0].attachment = 0;                                        // u32_t   attachment;
    Subpass1ColorAttachment[0].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VkImageLayout   layout;
    Subpass1ColorAttachment[1].attachment = 1;                                        // u32_t   attachment;
    Subpass1ColorAttachment[1].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VkImageLayout   layout;
    DepthAttachmentReference.attachment   = 3;                                        // u32_t   attachment;
    DepthAttachmentReference.layout       = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // VkImageLayout   layout;

    Subpass1->pipelineBindPoint        = VK_PIPELINE_BIND_POINT_GRAPHICS; // pipelineBindPoint   pipelineBindPoint
    Subpass1->colorAttachmentCount     = ArrayCount(Subpass1ColorAttachment); // colorAttachmentCount   colorAttachmentCount
    Subpass1->pColorAttachments        = &Subpass1ColorAttachment[0]; // VkAttachmentReference * pColorAttachments
    Subpass1->pDepthStencilAttachment  = &DepthAttachmentReference; // VkAttachmentReference * pDepthStencilAttachment

    /* BEGIN SUBPASS 2*/
    VkAttachmentReference Subpass2InputAttachment[2] = {};
    VkAttachmentReference Subpass2ColorAttachment;

    Subpass2InputAttachment[0].attachment = 0;
    Subpass2InputAttachment[0].layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    Subpass2InputAttachment[1].attachment = 1;
    Subpass2InputAttachment[1].layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    Subpass2ColorAttachment.attachment    = 2;
    Subpass2ColorAttachment.layout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    Subpass2->pipelineBindPoint        = VK_PIPELINE_BIND_POINT_GRAPHICS; // pipelineBindPoint   pipelineBindPoint
    Subpass2->inputAttachmentCount     = ArrayCount(Subpass2InputAttachment); // inputAttachmentCount   inputAttachmentCount
    Subpass2->pInputAttachments        = &Subpass2InputAttachment[0]; // VkAttachmentReference * pInputAttachments
    Subpass2->colorAttachmentCount     = 1; // colorAttachmentCount   colorAttachmentCount
    Subpass2->pColorAttachments        = &Subpass2ColorAttachment; // VkAttachmentReference * pColorAttachments

    /* BEGIN DEPENDENCIES */
    VkSubpassDependency Dependencies[3] = {};
    VkSubpassDependency * DependencyExternal     = Dependencies + 0;
    VkSubpassDependency * DependencyWeightShader = Dependencies + 1;
    VkSubpassDependency * DependencyToRender     = Dependencies + 2;
    
    DependencyExternal->srcSubpass        = VK_SUBPASS_EXTERNAL;                             // srcSubpass srcSubpass
    DependencyExternal->dstSubpass        = 0;                                               // dstSubpass dstSubpass
    DependencyExternal->srcStageMask      = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;   // srcStageMask srcStageMask
    DependencyExternal->dstStageMask      = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;   // dstStageMask dstStageMask
    DependencyExternal->srcAccessMask     = 0;                                               // srcAccessMask srcAccessMask
    DependencyExternal->dstAccessMask     = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;            // dstAccessMask dstAccessMask
    
    DependencyWeightShader->srcSubpass    = 0;                                               // srcSubpass srcSubpass
    DependencyWeightShader->dstSubpass    = 1;                                               // dstSubpass dstSubpass
    DependencyWeightShader->srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;   // srcStageMask srcStageMask
    DependencyWeightShader->dstStageMask  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;           // dstStageMask dstStageMask
    DependencyWeightShader->srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;            // srcAccessMask srcAccessMask
    DependencyWeightShader->dstAccessMask = VK_ACCESS_SHADER_READ_BIT;                       // dstAccessMask dstAccessMask
    
    DependencyToRender->srcSubpass        = 1;                                               // srcSubpass srcSubpass
    DependencyToRender->dstSubpass        = VK_SUBPASS_EXTERNAL;                             // dstSubpass dstSubpass
    DependencyToRender->srcStageMask      = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;           // srcStageMask srcStageMask
    DependencyToRender->dstStageMask      = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT; // dstStageMask dstStageMask
    DependencyToRender->srcAccessMask     = VK_ACCESS_SHADER_READ_BIT;                       // srcAccessMask srcAccessMask
    DependencyToRender->dstAccessMask     = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;            // dstAccessMask dstAccessMask
    
    /* CREATE RENDER PASS */
    VkRenderPassCreateInfo RenderPassCreateInfo = {};
    RenderPassCreateInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO; // sType   sType
    RenderPassCreateInfo.attachmentCount = ArrayCount(Attachments); // attachmentCount   attachmentCount
    RenderPassCreateInfo.pAttachments    = &Attachments[0]; // VkAttachmentDescription * pAttachments
    RenderPassCreateInfo.subpassCount    = ArrayCount(Subpasses); // subpassCount   subpassCount
    RenderPassCreateInfo.pSubpasses      = &Subpasses[0]; // VkSubpassDescription * pSubpasses
    RenderPassCreateInfo.dependencyCount = ArrayCount(Dependencies); // dependencyCount   dependencyCount
    RenderPassCreateInfo.pDependencies   = &Dependencies[0]; // VkSubpassDependency * pDependencies

    VK_CHECK(vkCreateRenderPass(GlobalVulkan.PrimaryDevice,&RenderPassCreateInfo, 0, &GlobalVulkan.RenderPassTransparency));

    return 0;
}

i32
VulkanInitDefaultRenderPass()
{
    VkAttachmentDescription ColorAttachment = {};
    ColorAttachment.format         = GlobalVulkan.SwapchainImageFormat; // VkFormat   format;
    ColorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;             // VkSampleCountFlagBits   samples;
    ColorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;       // VkAttachmentLoadOp   loadOp;
    ColorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;      // VkAttachmentStoreOp   storeOp;
    ColorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   // VkAttachmentLoadOp   stencilLoadOp;
    ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // VkAttachmentStoreOp   stencilStoreOp;
    ColorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;         // VkImageLayout   initialLayout;
    //ColorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;   // VkImageLayout   finalLayout;
    ColorAttachment.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;   // VkImageLayout   finalLayout;

    VkAttachmentDescription DepthAttachment = ColorAttachment;
    DepthAttachment.format         = GlobalVulkan.PrimaryDepthBuffer->Format;                  // VkFormat format;
    DepthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;                        // VkImageLayout initialLayout;
    DepthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // VkImageLayout finalLayout;

    VkAttachmentDescription Attachments[2] = {
        ColorAttachment,
        DepthAttachment
    };

    VkSubpassDescription SubpassDescription = {};
    VkAttachmentReference ColorAttachmentReference;
    VkAttachmentReference DepthAttachmentReference;

    ColorAttachmentReference.attachment = 0;                                        // u32_t   attachment;
    ColorAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VkImageLayout   layout;
    DepthAttachmentReference.attachment = 1; // u32_t   attachment;
    DepthAttachmentReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // VkImageLayout   layout;

    SubpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS; // VkPipelineBindPoint pipelineBindPoint;
    SubpassDescription.colorAttachmentCount    = 1;                               // u32_t colorAttachmentCount;
    SubpassDescription.pColorAttachments       = &ColorAttachmentReference;       // Typedef * pColorAttachments;
    SubpassDescription.pDepthStencilAttachment = &DepthAttachmentReference;       // Typedef * pDepthStencilAttachment;

#if 1
    VkSubpassDependency SelfDependency;
    SelfDependency.srcSubpass      = 0; // uint32_t   srcSubpass;
    SelfDependency.dstSubpass      = 0; // uint32_t   dstSubpass;
    SelfDependency.srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; 
    SelfDependency.dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    SelfDependency.srcAccessMask   = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT; // VkAccessFlags   srcAccessMask;
    SelfDependency.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    SelfDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; // VkSelfDependencyFlags   dependencyFlags;
#endif

    VkRenderPassCreateInfo RenderPassCreateInfo;
    RenderPassCreateInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO; // VkStructureType sType;
    RenderPassCreateInfo.pNext           = 0;                                         // Void * pNext;
    RenderPassCreateInfo.flags           = 0;                                         // VkRenderPassCreateFlags flags;
    RenderPassCreateInfo.attachmentCount = ArrayCount(Attachments);                   // u32_t attachmentCount;
    RenderPassCreateInfo.pAttachments    = &Attachments[0];                           // Typedef * pAttachments;
    RenderPassCreateInfo.subpassCount    = 1;                                         // u32_t subpassCount;
    RenderPassCreateInfo.pSubpasses      = &SubpassDescription;                       // Typedef * pSubpasses;
#if 1
    RenderPassCreateInfo.dependencyCount = 1;                                         // u32_t dependencyCount;
    RenderPassCreateInfo.pDependencies   = &SelfDependency;                               // Typedef * pDependencies;
#else
    RenderPassCreateInfo.dependencyCount = 0;                                         // u32_t dependencyCount;
    RenderPassCreateInfo.pDependencies   = 0;                               // Typedef * pDependencies;
#endif

    VK_CHECK(vkCreateRenderPass(GlobalVulkan.PrimaryDevice,&RenderPassCreateInfo, 0, &GlobalVulkan.RenderPass));

    return 0;
}

i32
VulkanInitFramebuffers()
{

    // 2 dependencies
    // swapchain
    // depth buffer
    for (u32 ImageIndex = 0;
                ImageIndex < GlobalVulkan.SwapchainImageCount;
                ++ImageIndex)
    {
        VkImageView Attachments[2] = {
            GlobalVulkan.SwapchainImageViews[ImageIndex],
            GlobalVulkan.PrimaryDepthBuffer->ImageView
        };

        VkFramebufferCreateInfo FramebufferCreateInfo;
        FramebufferCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;     // VkStructureType   sType;
        FramebufferCreateInfo.pNext           = 0;                                   // Void * pNext;
        FramebufferCreateInfo.flags           = 0;                                   // VkFramebufferCreateFlags flags;
        FramebufferCreateInfo.renderPass      = GlobalVulkan.RenderPass;             // VkRenderPass renderPass;
        FramebufferCreateInfo.attachmentCount = ArrayCount(Attachments);             // u32_t attachmentCount;
        FramebufferCreateInfo.pAttachments    = &Attachments[0];                     // Typedef * pAttachments;
        FramebufferCreateInfo.width           = GlobalVulkan.WindowExtension.width;  // u32_t width;
        FramebufferCreateInfo.height          = GlobalVulkan.WindowExtension.height; // u32_t height;
        FramebufferCreateInfo.layers          = 1;                                   // u32_t layers;

        VK_CHECK( vkCreateFramebuffer(GlobalVulkan.PrimaryDevice, &FramebufferCreateInfo, 0, &GlobalVulkan.Framebuffers[ImageIndex]));
    }

    for (u32 ImageIndex = 0;
                ImageIndex < GlobalVulkan.SwapchainImageCount;
                ++ImageIndex)
    {
        VkImageView Attachments[] = {
            GlobalVulkan.WeightedColorImage->ImageView,
            GlobalVulkan.WeightedRevealImage->ImageView,
            GlobalVulkan.SwapchainImageViews[ImageIndex],
            GlobalVulkan.PrimaryDepthBuffer->ImageView
        };

        VkFramebufferCreateInfo FramebufferCreateInfo;
        FramebufferCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;     // VkStructureType   sType;
        FramebufferCreateInfo.pNext           = 0;                                   // Void * pNext;
        FramebufferCreateInfo.flags           = 0;                                   // VkFramebufferCreateFlags flags;
        FramebufferCreateInfo.renderPass      = GlobalVulkan.RenderPassTransparency;             // VkRenderPass renderPass;
        FramebufferCreateInfo.attachmentCount = ArrayCount(Attachments);             // u32_t attachmentCount;
        FramebufferCreateInfo.pAttachments    = &Attachments[0];                     // Typedef * pAttachments;
        FramebufferCreateInfo.width           = GlobalVulkan.WindowExtension.width;  // u32_t width;
        FramebufferCreateInfo.height          = GlobalVulkan.WindowExtension.height; // u32_t height;
        FramebufferCreateInfo.layers          = 1;                                   // u32_t layers;

        VK_CHECK( vkCreateFramebuffer(GlobalVulkan.PrimaryDevice, &FramebufferCreateInfo, 0, &GlobalVulkan.FramebuffersTransparency[ImageIndex]));
    }

    for (u32 ImageIndex = 0;
                ImageIndex < GlobalVulkan.SwapchainImageCount;
                ++ImageIndex)
    {

        VULKAN_TREE_APPEND(vkDestroyFramebuffer,GlobalVulkan.Swapchain, GlobalVulkan.Framebuffers[ImageIndex]);
        VULKAN_TREE_APPEND(vkDestroyFramebuffer,GlobalVulkan.Swapchain, GlobalVulkan.FramebuffersTransparency[ImageIndex]);
    }


    return 0;
}



VkCommandBuffer
BeginSingleCommandBuffer()
{
    VkResult Result;

    VkCommandBufferAllocateInfo CommandBufferAllocateInfo;

    CommandBufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; // VkStructureType   sType;
    CommandBufferAllocateInfo.pNext              = 0;                               // Void * pNext;
    CommandBufferAllocateInfo.commandPool        = GetCurrentFrame()->CommandPool;
    CommandBufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // VkCommandBufferLevel level;
    CommandBufferAllocateInfo.commandBufferCount = 1;              // u32_t commandBufferCount;

    VkCommandBuffer CommandBuffer;

    Result = vkAllocateCommandBuffers(GlobalVulkan.PrimaryDevice, &CommandBufferAllocateInfo , &CommandBuffer);

    if (VK_PASSES(Result))
    {

        VkCommandBufferBeginInfo CommandBufferBeginInfo;
        CommandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; // VkStructureType   sType;
        CommandBufferBeginInfo.pNext            = 0;                                           // Void * pNext;
        CommandBufferBeginInfo.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // VkCommandBufferUsageFlags   flags;
        CommandBufferBeginInfo.pInheritanceInfo = 0;                                           // Typedef * pInheritanceInfo;

        Result = vkBeginCommandBuffer(CommandBuffer,&CommandBufferBeginInfo);
        if (VK_FAILS(Result))
        {
            Assert(0);
            vkFreeCommandBuffers(GlobalVulkan.PrimaryDevice, GetCurrentFrame()->CommandPool, 1, &CommandBuffer);
            CommandBuffer = VK_NULL_HANDLE;
        }
    }

    return CommandBuffer;
}

i32
EndSingleCommandBuffer(VkCommandBuffer CommandBuffer)
{
    vkEndCommandBuffer(CommandBuffer);

    VkSubmitInfo SubmitInfo = {};
    SubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO; // VkStructureType   sType;
    SubmitInfo.pNext                = 0; // Void * pNext;
    SubmitInfo.waitSemaphoreCount   = 0; // u32_t   waitSemaphoreCount;
    SubmitInfo.pWaitSemaphores      = 0; // Typedef * pWaitSemaphores;
    SubmitInfo.pWaitDstStageMask    = 0; // Typedef * pWaitDstStageMask;
    SubmitInfo.commandBufferCount   = 1; // u32_t   commandBufferCount;
    SubmitInfo.pCommandBuffers      = &CommandBuffer; // Typedef * pCommandBuffers;
    SubmitInfo.signalSemaphoreCount = 0; // u32_t   signalSemaphoreCount;
    SubmitInfo.pSignalSemaphores    = 0; // Typedef * pSignalSemaphores;

    VK_CHECK(vkQueueSubmit(GlobalVulkan.GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE));
    vkQueueWaitIdle(GlobalVulkan.GraphicsQueue);

    vkFreeCommandBuffers(GlobalVulkan.PrimaryDevice, GetCurrentFrame()->CommandPool, 1, &CommandBuffer);

    return 0;
}

void
RenderPushVertexConstant(u32 Size,void * Data)
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;

    vkCmdPushConstants(cmd,GlobalVulkan.CurrentPipelineLayout,
                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                        0,Size,Data);
}


VkSamplerCreateInfo
VulkanSamplerCreateInfo(VkFilter Filters,VkSamplerAddressMode SamplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT)
{
    VkSamplerCreateInfo SamplerCreateInfo;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(GlobalVulkan.PrimaryGPU, &properties);
    r32 MaxSamplerAnisotropy = properties.limits.maxSamplerAnisotropy;

    SamplerCreateInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO; // VkStructureType   sType;
    SamplerCreateInfo.pNext                   = 0; // Void * pNext;
    SamplerCreateInfo.flags                   = 0; // VkSamplerCreateFlags   flags;
    SamplerCreateInfo.magFilter               = Filters; // VkFilter   magFilter;
    SamplerCreateInfo.minFilter               = Filters; // VkFilter   minFilter;
    SamplerCreateInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR; // VkSamplerMipmapMode   mipmapMode;
    SamplerCreateInfo.addressModeU            = SamplerAddressMode; // VkSamplerAddressMode   addressModeU;
    SamplerCreateInfo.addressModeV            = SamplerAddressMode; // VkSamplerAddressMode   addressModeV;
    SamplerCreateInfo.addressModeW            = SamplerAddressMode; // VkSamplerAddressMode   addressModeW;
    SamplerCreateInfo.mipLodBias              = 0.0f; // FLOAT   mipLodBias;
    SamplerCreateInfo.anisotropyEnable        = VK_TRUE; // VkBool32   anisotropyEnable;
    SamplerCreateInfo.maxAnisotropy           = MaxSamplerAnisotropy; // FLOAT   maxAnisotropy;
    SamplerCreateInfo.compareEnable           = VK_FALSE; // VkBool32   compareEnable;
    SamplerCreateInfo.compareOp               = VK_COMPARE_OP_ALWAYS; // VkCompareOp   compareOp;
    SamplerCreateInfo.minLod                  = 0.0f; // FLOAT   minLod;
    SamplerCreateInfo.maxLod                  = 0.0f; // FLOAT   maxLod;
    SamplerCreateInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // VkBorderColor   borderColor;
    SamplerCreateInfo.unnormalizedCoordinates = VK_FALSE; // VkBool32   unnormalizedCoordinates;

    return SamplerCreateInfo;
}


VkWriteDescriptorSet
VH_WriteDescriptorImage(u32 BindingSlot,VkDescriptorSet Set,VkDescriptorType DescriptorType, VkDescriptorImageInfo * ImageInfo, u32 ArraySize)
{
    VkWriteDescriptorSet WriteDescriptor;

    WriteDescriptor.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; // VkStructureType   sType;
    WriteDescriptor.pNext            = 0; // Void * pNext;
    WriteDescriptor.dstSet           = Set; // VkDescriptorSet   dstSet;
    WriteDescriptor.dstBinding       = BindingSlot; // uint32_t   dstBinding;
    WriteDescriptor.dstArrayElement  = 0; // uint32_t   dstArrayElement;
    WriteDescriptor.descriptorCount  = ArraySize; // uint32_t   descriptorCount;
    WriteDescriptor.descriptorType   = DescriptorType;
    WriteDescriptor.pImageInfo       = ImageInfo; // Typedef * pImageInfo;
    WriteDescriptor.pBufferInfo      = 0; // Typedef * pBufferInfo;
    WriteDescriptor.pTexelBufferView = 0; // Typedef * pTexelBufferView;

    return WriteDescriptor;
}

i32
VulkanWriteDataToArena(gpu_arena * Arena, void * Data, u32 Size)
{
    i32 Error = VulkanMapArena(Arena);
    if (Error == 0)
    {
        memcpy(Arena->WriteToAddr, Data, Size);
        VulkanUnmapArena(Arena);
    }

    return Error;
}

i32
VH_BeginCommandBuffer(VkCommandBuffer CommandBuffer)
{
    VkCommandBufferBeginInfo CommandBufferBeginInfo;
    CommandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; // VkStructureType   sType;
    CommandBufferBeginInfo.pNext            = 0;                                           // Void * pNext;
    CommandBufferBeginInfo.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // VkCommandBufferUsageFlags   flags;
    CommandBufferBeginInfo.pInheritanceInfo = 0;                                           // Typedef * pInheritanceInfo;

    VK_CHECK(vkResetCommandBuffer(CommandBuffer, 0));
    VK_CHECK(vkBeginCommandBuffer(CommandBuffer,&CommandBufferBeginInfo));

    return 0;
}

i32
VH_EndCommandBuffer(VkCommandBuffer CommandBuffer, VkQueue FamilyQueue)
{
    vkEndCommandBuffer(CommandBuffer);

    VkSubmitInfo SubmitInfo = {};
    SubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO; // VkStructureType   sType;
    SubmitInfo.pNext                = 0; // Void * pNext;
    SubmitInfo.waitSemaphoreCount   = 0; // u32_t   waitSemaphoreCount;
    SubmitInfo.pWaitSemaphores      = 0; // Typedef * pWaitSemaphores;
    SubmitInfo.pWaitDstStageMask    = 0; // Typedef * pWaitDstStageMask;
    SubmitInfo.commandBufferCount   = 1; // u32_t   commandBufferCount;
    SubmitInfo.pCommandBuffers      = &CommandBuffer; // Typedef * pCommandBuffers;
    SubmitInfo.signalSemaphoreCount = 0; // u32_t   signalSemaphoreCount;
    SubmitInfo.pSignalSemaphores    = 0; // Typedef * pSignalSemaphores;

    VK_CHECK(vkQueueSubmit(FamilyQueue, 1, &SubmitInfo, VK_NULL_HANDLE));
    vkQueueWaitIdle(FamilyQueue);

    return 0;
}

i32
VH_CopyBuffer(VkCommandBuffer CommandBuffer, VkBuffer Src, VkBuffer Dest, VkDeviceSize Size, VkDeviceSize Offset)
{
    VH_BeginCommandBuffer(CommandBuffer);

    VkBufferCopy CopyRegion = {};
    CopyRegion.srcOffset = 0;    // VkDeviceSize srcOffset;
    CopyRegion.dstOffset = Offset;    // VkDeviceSize dstOffset;
    CopyRegion.size      = Size; // VkDeviceSize size;

    vkCmdCopyBuffer(CommandBuffer, Src, Dest, 1, &CopyRegion);

    VH_EndCommandBuffer(CommandBuffer,GlobalVulkan.TransferOnlyQueue);

    return 0;
}

i32
VH_AllocateDescriptor(VkDescriptorSetLayout  * SetLayout,
                   VkDescriptorPool Pool,
                   VkDescriptorSet * Set)
{
    VkDescriptorSetAllocateInfo AllocInfo;
    AllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO; // VkStructureType   sType;
    AllocInfo.pNext              = 0; // Void * pNext;
    AllocInfo.descriptorPool     = Pool; // VkDescriptorPool   descriptorPool;
    AllocInfo.descriptorSetCount = 1; // uint32_t   descriptorSetCount;
    AllocInfo.pSetLayouts        = SetLayout; // Typedef * pSetLayouts;

    vkAllocateDescriptorSets(GlobalVulkan.PrimaryDevice, &AllocInfo, Set);

    return 0;
}
#if 0
i32 
PushTextureData(void * Data, u32 Width, u32 Height, u32 Channels)
{

    i32 ResultImageIndex = -1;

    gpu_arena * Arena = GlobalVulkan.TextureArena;
    Assert(Arena->ImageCount < ArrayCount(Arena->Images));
    vulkan_image * VulkanImage = Arena->Images + Arena->ImageCount;

    VkDeviceSize DataSize = Width * Height * Channels;
    VkDeviceSize Align = Arena->Alignment - 1;
    u32 DataSizeAligned = (u32)((DataSize + Align) & ~Align);

    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkMemoryMapFlags.html

    if (VH_CreateImage(GlobalVulkan.PrimaryDevice, Width,Height,Channels, VulkanImage)) 
        return ResultImageIndex;

    VkDeviceMemory DeviceMemory = GetDeviceMemory(Arena->MemoryIndexType);
    VkDeviceSize BindMemoryOffset = Arena->DeviceBindingOffsetBegin + Arena->CurrentSize;

    Assert( (Arena->MaxSize - Arena->CurrentSize) > DataSizeAligned);

    if (VK_FAILS(vkBindImageMemory(GlobalVulkan.PrimaryDevice, VulkanImage->Image,DeviceMemory, BindMemoryOffset)))
    {
        VH_DestroyImage(Arena->Device,VulkanImage);
        return ResultImageIndex;
    }

    /*
     * 1) Copy CPU to GPU visible memory
     * 2) Transition Image as destination
     * 3) Copy GPU temp memory to Image
     * 4) Transition Image to readable by shaders
     */
    VulkanWriteDataToArena(GlobalVulkan.TransferBitArena, Data, (u32)DataSizeAligned);

    /* TRANSITION LAYOUT */

    VkImageSubresourceRange Range;
    Range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    Range.baseMipLevel   = 0; // uint32_t baseMipLevel;
    Range.levelCount     = 1; // uint32_t levelCount;
    Range.baseArrayLayer = 0; // uint32_t baseArrayLayer;
    Range.layerCount     = 1; // uint32_t layerCount;

    VkImageMemoryBarrier TransferBarrier = {};
    TransferBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    TransferBarrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    TransferBarrier.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    TransferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    TransferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    TransferBarrier.image               = VulkanImage->Image;
    TransferBarrier.subresourceRange    = Range;

    TransferBarrier.srcAccessMask = 0;
    TransferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    VkCommandBuffer SingleCmd = BeginSingleCommandBuffer();

    Assert(VK_VALID_HANDLE(SingleCmd));

    vkCmdPipelineBarrier(SingleCmd, 
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
                            VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &TransferBarrier);

    EndSingleCommandBuffer(SingleCmd);

    /* COPY IMAGE */
    VkBufferImageCopy Copy;

    VkImageSubresourceLayers   ImageSubresource;
    ImageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT; // VkImageAspectFlags   aspectMask;
    ImageSubresource.mipLevel       = 0; // uint32_t   mipLevel;
    ImageSubresource.baseArrayLayer = 0; // uint32_t   baseArrayLayer;
    ImageSubresource.layerCount     = 1; // uint32_t   layerCount;

    VkExtent3D   ImageExtent;
    ImageExtent.width  = Width; // uint32_t   width;
    ImageExtent.height = Height; // uint32_t   height;
    ImageExtent.depth  = 1; // uint32_t   depth;

    VkOffset3D ImageOffset;
    ImageOffset.x = 0; // int32_t   x;
    ImageOffset.y = 0; // int32_t   y;
    ImageOffset.z = 0; // int32_t   z;

    Copy.bufferOffset      = 0; // VkDeviceSize   bufferOffset;
    Copy.bufferRowLength   = 0; // uint32_t   bufferRowLength;
    Copy.bufferImageHeight = 0; // uint32_t   bufferImageHeight;
    Copy.imageSubresource  = ImageSubresource; // VkImageSubresourceLayers   imageSubresource;
    Copy.imageOffset       = ImageOffset; // VkOffset3D   imageOffset;
    Copy.imageExtent       = ImageExtent; // VkExtent3D   imageExtent;

    VH_BeginCommandBuffer(GlobalVulkan.TransferBitCommandBuffer);


	vkCmdCopyBufferToImage(GlobalVulkan.TransferBitCommandBuffer, 
                           GlobalVulkan.TransferBitArena->Buffer, 
                           VulkanImage->Image, 
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Copy);

    VH_EndCommandBuffer(GlobalVulkan.TransferBitCommandBuffer, GlobalVulkan.TransferOnlyQueue);

    /* TRANSITION LAYOUT */
    VkImageMemoryBarrier ReadableBarrier = {};
    ReadableBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ReadableBarrier.oldLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    ReadableBarrier.newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ReadableBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ReadableBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ReadableBarrier.image               = VulkanImage->Image;
    ReadableBarrier.subresourceRange    = Range;

    ReadableBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    ReadableBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    SingleCmd = BeginSingleCommandBuffer();

    vkCmdPipelineBarrier(SingleCmd, 
                            VK_PIPELINE_STAGE_TRANSFER_BIT, 
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &ReadableBarrier);

    EndSingleCommandBuffer(SingleCmd);

    VkImageViewCreateInfo ImageViewCreateInfo;

    ImageViewCreateInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; // VkStructureType   sType;
    ImageViewCreateInfo.pNext            = 0; // Void * pNext;
    ImageViewCreateInfo.flags            = 0; // VkImageViewCreateFlags   flags;
    ImageViewCreateInfo.image            = VulkanImage->Image; // VkImage   image;
    ImageViewCreateInfo.viewType         = VK_IMAGE_VIEW_TYPE_2D; // VkImageViewType   viewType;
    ImageViewCreateInfo.format           = VulkanImage->Format; // VkFormat   format;

    VkComponentMapping   components;
    components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // VkComponentSwizzle   r;
    components.g = VK_COMPONENT_SWIZZLE_IDENTITY; // VkComponentSwizzle   g;
    components.b = VK_COMPONENT_SWIZZLE_IDENTITY; // VkComponentSwizzle   b;
    components.a = VK_COMPONENT_SWIZZLE_IDENTITY; // VkComponentSwizzle   a;
    ImageViewCreateInfo.components       = components; // VkComponentMapping   components;

    VkImageSubresourceRange   subresourceRange;
    subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT; // VkImageAspectFlags   aspectMask;
    subresourceRange.baseMipLevel   = 0; // uint32_t   baseMipLevel;
    subresourceRange.levelCount     = 1; // uint32_t   levelCount;
    subresourceRange.baseArrayLayer = 0; // uint32_t   baseArrayLayer;
    subresourceRange.layerCount     = 1; // uint32_t   layerCount;

    ImageViewCreateInfo.subresourceRange = subresourceRange; // VkImageSubresourceRange   subresourceRange;

    VK_CHECK(vkCreateImageView( GlobalVulkan.PrimaryDevice, &ImageViewCreateInfo, 0, &VulkanImage->ImageView));

    Arena->CurrentSize += maxval((u32)VulkanImage->MemoryRequirements.size, DataSizeAligned);

    // commit
    ResultImageIndex = Arena->ImageCount++;

    GlobalVulkan.TextureArenaDirty = true;

    return ResultImageIndex;
}
#endif

i32 
PushTextureData(u32 ID, void * Data, u32 Width, u32 Height, u32 Channels)
{

    i32 ResultImageIndex = -1;

    gpu_heap * HeapAlloc = GlobalVulkan.TextureHeap;

    VkDeviceSize DataSize        = Width * Height * Channels;
    VkDeviceSize Align           = HeapAlloc->Alignment - 1;
    u32          DataSizeAligned = (u32)((DataSize + Align) & ~Align);

    gpu_heap_block * Block = FindHeapBlock(HeapAlloc, DataSizeAligned);

    vulkan_image * VulkanImage = &Block->Image;

    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkMemoryMapFlags.html

    if (VH_CreateImage(GlobalVulkan.PrimaryDevice, Width,Height,Channels, VulkanImage)) 
        return ResultImageIndex;

    VkDeviceMemory   DeviceMemory     = GetDeviceMemory(HeapAlloc->MemoryIndexType);
    VkDeviceSize     BindMemoryOffset = HeapAlloc->DeviceBindingOffsetBegin + Block->Begin;

    if (VK_FAILS(vkBindImageMemory(GlobalVulkan.PrimaryDevice, VulkanImage->Image,DeviceMemory, BindMemoryOffset)))
    {
        VH_DestroyImage(HeapAlloc->Device,VulkanImage);
        return ResultImageIndex;
    }

    /*
     * 1) Copy CPU to GPU visible memory
     * 2) Transition Image as destination
     * 3) Copy GPU temp memory to Image
     * 4) Transition Image to readable by shaders
     */
    VulkanWriteDataToArena(GlobalVulkan.TransferBitArena, Data, (u32)DataSizeAligned);

    /* TRANSITION LAYOUT */

    VkImageSubresourceRange Range;
    Range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    Range.baseMipLevel   = 0; // uint32_t baseMipLevel;
    Range.levelCount     = 1; // uint32_t levelCount;
    Range.baseArrayLayer = 0; // uint32_t baseArrayLayer;
    Range.layerCount     = 1; // uint32_t layerCount;

    VkImageMemoryBarrier TransferBarrier = {};
    TransferBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    TransferBarrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    TransferBarrier.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    TransferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    TransferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    TransferBarrier.image               = VulkanImage->Image;
    TransferBarrier.subresourceRange    = Range;

    TransferBarrier.srcAccessMask = 0;
    TransferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    VkCommandBuffer SingleCmd = BeginSingleCommandBuffer();

    Assert(VK_VALID_HANDLE(SingleCmd));

    vkCmdPipelineBarrier(SingleCmd, 
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
                            VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &TransferBarrier);

    EndSingleCommandBuffer(SingleCmd);

    /* COPY IMAGE */
    VkBufferImageCopy Copy;

    VkImageSubresourceLayers   ImageSubresource;
    ImageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT; // VkImageAspectFlags   aspectMask;
    ImageSubresource.mipLevel       = 0; // uint32_t   mipLevel;
    ImageSubresource.baseArrayLayer = 0; // uint32_t   baseArrayLayer;
    ImageSubresource.layerCount     = 1; // uint32_t   layerCount;

    VkExtent3D   ImageExtent;
    ImageExtent.width  = Width; // uint32_t   width;
    ImageExtent.height = Height; // uint32_t   height;
    ImageExtent.depth  = 1; // uint32_t   depth;

    VkOffset3D ImageOffset;
    ImageOffset.x = 0; // int32_t   x;
    ImageOffset.y = 0; // int32_t   y;
    ImageOffset.z = 0; // int32_t   z;

    Copy.bufferOffset      = 0; // VkDeviceSize   bufferOffset;
    Copy.bufferRowLength   = 0; // uint32_t   bufferRowLength;
    Copy.bufferImageHeight = 0; // uint32_t   bufferImageHeight;
    Copy.imageSubresource  = ImageSubresource; // VkImageSubresourceLayers   imageSubresource;
    Copy.imageOffset       = ImageOffset; // VkOffset3D   imageOffset;
    Copy.imageExtent       = ImageExtent; // VkExtent3D   imageExtent;

    VH_BeginCommandBuffer(GlobalVulkan.TransferBitCommandBuffer);


	vkCmdCopyBufferToImage(GlobalVulkan.TransferBitCommandBuffer, 
                           GlobalVulkan.TransferBitArena->Buffer, 
                           VulkanImage->Image, 
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Copy);

    VH_EndCommandBuffer(GlobalVulkan.TransferBitCommandBuffer, GlobalVulkan.TransferOnlyQueue);

    /* TRANSITION LAYOUT */
    VkImageMemoryBarrier ReadableBarrier = {};
    ReadableBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ReadableBarrier.oldLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    ReadableBarrier.newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ReadableBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ReadableBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ReadableBarrier.image               = VulkanImage->Image;
    ReadableBarrier.subresourceRange    = Range;

    ReadableBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    ReadableBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    SingleCmd = BeginSingleCommandBuffer();

    vkCmdPipelineBarrier(SingleCmd, 
                            VK_PIPELINE_STAGE_TRANSFER_BIT, 
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &ReadableBarrier);

    EndSingleCommandBuffer(SingleCmd);

    VkImageViewCreateInfo ImageViewCreateInfo;

    ImageViewCreateInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; // VkStructureType   sType;
    ImageViewCreateInfo.pNext            = 0; // Void * pNext;
    ImageViewCreateInfo.flags            = 0; // VkImageViewCreateFlags   flags;
    ImageViewCreateInfo.image            = VulkanImage->Image; // VkImage   image;
    ImageViewCreateInfo.viewType         = VK_IMAGE_VIEW_TYPE_2D; // VkImageViewType   viewType;
    ImageViewCreateInfo.format           = VulkanImage->Format; // VkFormat   format;

    VkComponentMapping   components;
    components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // VkComponentSwizzle   r;
    components.g = VK_COMPONENT_SWIZZLE_IDENTITY; // VkComponentSwizzle   g;
    components.b = VK_COMPONENT_SWIZZLE_IDENTITY; // VkComponentSwizzle   b;
    components.a = VK_COMPONENT_SWIZZLE_IDENTITY; // VkComponentSwizzle   a;
    ImageViewCreateInfo.components       = components; // VkComponentMapping   components;

    VkImageSubresourceRange   subresourceRange;
    subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT; // VkImageAspectFlags   aspectMask;
    subresourceRange.baseMipLevel   = 0; // uint32_t   baseMipLevel;
    subresourceRange.levelCount     = 1; // uint32_t   levelCount;
    subresourceRange.baseArrayLayer = 0; // uint32_t   baseArrayLayer;
    subresourceRange.layerCount     = 1; // uint32_t   layerCount;

    ImageViewCreateInfo.subresourceRange = subresourceRange; // VkImageSubresourceRange   subresourceRange;

    VK_CHECK(vkCreateImageView( GlobalVulkan.PrimaryDevice, &ImageViewCreateInfo, 0, &VulkanImage->ImageView));

    ResultImageIndex = Block->InternalID;

    Block->ExternalID = ID;

    vulkan_hash_table_entry * Entry = 
        (vulkan_hash_table_entry *)AddEntryToTable(&GlobalVulkan.HeapAssetsHashTable, &ID);
    Entry->Block    = Block;
    Entry->AssetID  = ID;

    GlobalVulkan.TextureArenaDirty = true;

    return ResultImageIndex;
}

HASH_TABLE_MAP_FUNCTION(HeapHashTableMap)
{
    u32 * ID = (u32 *)Data;

    u32 HashKey = *ID;

    return HashKey;
}

HASH_TABLE_COMPARE_FUNCTION(HeapHashTableCompare)
{
    vulkan_hash_table_entry * EntryA = (vulkan_hash_table_entry *)Entry;
    vulkan_hash_table_entry * EntryB = (vulkan_hash_table_entry *)EntryLookup;

    i32 Result = (i32)(EntryA->AssetID - EntryB->AssetID);

    return Result;
}

void
InitializeVulanHashTable(memory_arena * Arena)
{
    u32 EntriesCount = 5;
    u32 HashBucketCount = 200;
    
    GlobalVulkan.HeapAssetsHashTable = 
        InitializeHashTable(Arena, 
                            vulkan_hash_table_entry, EntriesCount,
                            HeapHashTableMap,
                            HeapHashTableCompare,
                            HashBucketCount);
}

internal i32 
PushVertexData(void * Data, u32 DataSize, i32 MeshID)
{
    gpu_heap       * HeapAlloc = GlobalVulkan.VertexHeap;
    gpu_heap_block * Block     = FindHeapBlock(HeapAlloc, DataSize);

    // buffers are binded to offset location
    //VkDeviceSize Offset = HeapAlloc->DeviceBindingOffsetBegin + Block->Begin;
    VkDeviceSize Offset = Block->Begin;

    Assert(GlobalVulkan.TransferBitArena->MaxSize >= Block->Size);

    VulkanWriteDataToArena(GlobalVulkan.TransferBitArena,Data, DataSize);

    if (VH_CopyBuffer(GlobalVulkan.TransferBitCommandBuffer, 
                     GlobalVulkan.TransferBitArena->Buffer,HeapAlloc->Buffer, DataSize, Offset))
    {
        Log("Failed to copy data from buffer to gpu\n");
        return 1;
    }

    Block->ExternalID = MeshID;

    return 0;
}

internal i32
PushIndexData(void * Data,u32 DataSize, i32 MeshID)
{
    gpu_heap       * HeapAlloc = GlobalVulkan.IndexHeap;
    gpu_heap_block * Block     = FindHeapBlock(HeapAlloc, DataSize);

    VkDeviceSize Offset = Block->Begin;

    Assert(GlobalVulkan.TransferBitArena->MaxSize >= Block->Size);

    VulkanWriteDataToArena(GlobalVulkan.TransferBitArena,Data, DataSize);

    if (VH_CopyBuffer(GlobalVulkan.TransferBitCommandBuffer, 
                     GlobalVulkan.TransferBitArena->Buffer,HeapAlloc->Buffer, DataSize, Offset))
    {
        Log("Failed to copy data from buffer to gpu\n");
        return 1;
    }

    Block->ExternalID = MeshID;

    return 0;
}

i32
PushMeshData(i32 CustomMeshID, void * VertexData, u32 VertexSize, void * IndicesData, u32 IndicesSize)
{
    i32 ErrorCode = PushVertexData(VertexData, VertexSize   , CustomMeshID);
    if (ErrorCode == 0)
    {
        ErrorCode = PushIndexData(IndicesData, IndicesSize  , CustomMeshID);
    }

    return ErrorCode;
}

i32
ReleaseGPUMemory(i32 AssetID)
{
    gpu_heap * Heap = GlobalVulkan.VertexHeap;
    gpu_heap_block * Block = FindHeapBlockByID(Heap  , AssetID);
    if (Block)
    {
        ReleaseBlock(Heap, Block);
        Heap = GlobalVulkan.IndexHeap;
        Block  = FindHeapBlockByID(Heap   , AssetID);
        Assert(Block);
        ReleaseBlock(Heap, Block);
    }
    else
    {
        Heap = GlobalVulkan.TextureHeap;
        Block  = FindHeapBlockByID(Heap   , AssetID);
        Assert(Block);
        vulkan_image * Image = &Block->Image;
        VH_DestroyImage(Heap->Device,Image);
        Image->Format    = {};
        Image->ImageView = VK_NULL_HANDLE;
        Image->Image     = VK_NULL_HANDLE;
    }


    return 0;
}


i32 
CreateShaderModule(void * Buffer, size_t Size)
{

    i32 ShaderIndex;
    VkShaderModule * ShaderModule = 0;

    for (ShaderIndex = 0; ShaderIndex < (i32)ArrayCount(GlobalVulkan.ShaderModules); ++ShaderIndex)
    {
        ShaderModule = GlobalVulkan.ShaderModules + ShaderIndex;
        if ((*ShaderModule) == VK_NULL_HANDLE)
        {
            break;
        }
    }

    // not enough cache? sync with cpu assest load model
    Assert(ShaderIndex < (i32)ArrayCount(GlobalVulkan.ShaderModules));

    VkShaderModuleCreateInfo ShaderModuleCreateInfo;

    ShaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO; // VkStructureType sType;
    ShaderModuleCreateInfo.pNext    = 0;                                           // Void * pNext;
    ShaderModuleCreateInfo.flags    = 0;                                           // VkShaderModuleCreateFlags flags;
    ShaderModuleCreateInfo.codeSize = Size;                                        // size_t codeSize;
    ShaderModuleCreateInfo.pCode    = (u32 *)Buffer;                            // Typedef * pCode;

    if (VK_FAILS(vkCreateShaderModule(GlobalVulkan.PrimaryDevice, &ShaderModuleCreateInfo, 0, ShaderModule)))
    {
        ShaderIndex = -1;
        ShaderModule = VK_NULL_HANDLE;
    }

    VULKAN_TREE_APPEND(vkDestroyShaderModule, GlobalVulkan.ShaderModulesLabel, GlobalVulkan.ShaderModules[ShaderIndex]);

    return ShaderIndex;
}

i32
VulkanSetCurrentImageSwap()
{
    u32 SwapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(GlobalVulkan.PrimaryDevice, GlobalVulkan.Swapchain, 1000000000, GetCurrentFrame()->ImageAvailableSemaphore, 0 , &SwapchainImageIndex));
    GlobalVulkan.CurrentSwapchainImageIndex = SwapchainImageIndex;

    return 0;
}

void
UpdateTextureDescriptorBinding()
{
    u32 TextureCount = GlobalVulkan.TextureHeap->BlockCount;

    for (u32 BlockIndex = 0; 
             BlockIndex < TextureCount;
             ++BlockIndex)
    {
        VkDescriptorImageInfo * ImageInfo   = GlobalVulkan.ImageInfoArray + BlockIndex;
        vulkan_image          * VulkanImage = &GlobalVulkan.TextureHeap->Blocks[BlockIndex].Image;

        ImageInfo->sampler     = 0;
        ImageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // VkImageLayout   imageLayout;
        ImageInfo->imageView   = VulkanImage->ImageView;

        Assert(VulkanImage->ImageView != VK_NULL_HANDLE);
    }

    i32 UnusedImageInfoCount = GPU_TEXTURES_ARRAY_COUNT - TextureCount;
    RtlZeroMemory(GlobalVulkan.ImageInfoArray + TextureCount, UnusedImageInfoCount * sizeof(VkDescriptorImageInfo));

    VkWriteDescriptorSet TextImageWriteSet[2]; 
    VkDescriptorImageInfo SamplerInfo = {};
    SamplerInfo.sampler = GlobalVulkan.TextureSampler;

    TextImageWriteSet[0] = VH_WriteDescriptorImage(0, 
                                GlobalVulkan._DebugTextureSet, 
                                VK_DESCRIPTOR_TYPE_SAMPLER, 
                                &SamplerInfo, 1);
    TextImageWriteSet[1] = VH_WriteDescriptorImage(1, 
                                GlobalVulkan._DebugTextureSet, 
                                VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 
                                GlobalVulkan.ImageInfoArray , GPU_TEXTURES_ARRAY_COUNT);

    vkUpdateDescriptorSets(GlobalVulkan.PrimaryDevice,ArrayCount(TextImageWriteSet), TextImageWriteSet,  0, nullptr);
}


i32
RenderBeginPass(v4 ClearColor)
{
    if (GlobalWindowIsMinimized) return 0;


    if (VulkanSetCurrentImageSwap())
    {
        Log("Couldn't fetch current swapchain Image index\n");
        return 1;
    }

    u32 SwapchainImageIndex = GlobalVulkan.CurrentSwapchainImageIndex;
    VK_CHECK(vkResetCommandBuffer(GetCurrentFrame()->PrimaryCommandBuffer, 0));

    if (GlobalVulkan.TextureArenaDirty)
    {
        UpdateTextureDescriptorBinding();
        GlobalVulkan.TextureArenaDirty = false;
    }

    GetCurrentFrame()->ObjectsArena->CurrentSize = 0;

    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;

    VkCommandBufferBeginInfo CommandBufferBeginInfo;
    CommandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; // VkStructureType   sType;
    CommandBufferBeginInfo.pNext            = 0;                                           // Void * pNext;
    CommandBufferBeginInfo.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // VkCommandBufferUsageFlags   flags;
    CommandBufferBeginInfo.pInheritanceInfo = 0;                                           // Typedef * pInheritanceInfo;

    VK_CHECK(vkBeginCommandBuffer(cmd, &CommandBufferBeginInfo));

    VkClearValue ColorClear = {};
    ColorClear.color.float32[0] = ClearColor.r; // VkClearColorValue color;
    ColorClear.color.float32[1] = ClearColor.g; // VkClearColorValue color;
    ColorClear.color.float32[2] = ClearColor.b; // VkClearColorValue color;
    ColorClear.color.float32[3] = ClearColor.a; // VkClearColorValue color;
    //ClearValue.depthStencil  = {};            // VkClearDepthStencilValue depthStencil;
    VkClearValue DepthClear;
    DepthClear.depthStencil = {1.0f,0};

    VkClearValue ClearAttachments[2] = {
        ColorClear,
        DepthClear
    };

    VkRenderPassBeginInfo RenderPassBeginInfo = {};
    RenderPassBeginInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;       // VkStructureType   sType;
    RenderPassBeginInfo.pNext             = 0;                                              // Void * pNext;
    RenderPassBeginInfo.renderPass        = GlobalVulkan.RenderPass;                        // VkRenderPass renderPass;
    RenderPassBeginInfo.framebuffer       = GlobalVulkan.Framebuffers[SwapchainImageIndex]; // VkFramebuffer framebuffer;
    RenderPassBeginInfo.renderArea.extent = GlobalVulkan.WindowExtension;                   // VkRect2D renderArea;
    RenderPassBeginInfo.clearValueCount   = ArrayCount(ClearAttachments);                   // u32_t clearValueCount;
    RenderPassBeginInfo.pClearValues      = &ClearAttachments[0];                           // Typedef * pClearValues;

    vkCmdBeginRenderPass(cmd, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    GetCurrentFrame()->ObjectsCount = 0;

    u32 CurrentFrameIndex = GlobalVulkan._CurrentFrameData % FRAME_OVERLAP;
    u32 UniformOffset = (u32)VH_PaddedUniformBuffer(sizeof(GPUSimulationData)) * CurrentFrameIndex;

    VkPipelineLayout PipelineLayout = GlobalVulkan.PipelineLayout[0];

    GlobalVulkan.CurrentPipelineLayout = PipelineLayout;


    u32 UniformBufferBindingSlot  = 0;
    u32 ObjectDataBindingSlot     = 1;
    u32 TextureBindingSlot        = 2;

    // Global descriptor, bind once
    vkCmdBindDescriptorSets(cmd, 
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            PipelineLayout, 
                            UniformBufferBindingSlot, 1, 
                            &GetCurrentFrame()->GlobalDescriptor, 
                            1, 
                            &UniformOffset);

    vkCmdBindDescriptorSets(cmd, 
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            PipelineLayout, 
                            ObjectDataBindingSlot, 1, 
                            &GetCurrentFrame()->ObjectsDescriptor, 
                            0, nullptr);

    vkCmdBindDescriptorSets(GetCurrentFrame()->PrimaryCommandBuffer, 
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            PipelineLayout, 
                            TextureBindingSlot, 1, 
                            &GlobalVulkan._DebugTextureSet, 
                            0, nullptr);

    return 0;
}

i32
RenderSetPipeline(i32 PipelineIndex)
{
    Assert((PipelineIndex >= 0) && ((u32)PipelineIndex < GlobalVulkan.PipelinesCount));
    Assert(VK_VALID_HANDLE(GlobalVulkan.Pipelines[PipelineIndex]));

    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;

    VkPipeline Pipeline = GlobalVulkan.Pipelines[PipelineIndex];
    VkViewport Viewport = VH_CreateDefaultViewport(GlobalVulkan.WindowExtension);
    VkRect2D Scissor;
    Scissor.offset       = {0,0};
    Scissor.extent       = GlobalVulkan.WindowExtension;

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);
    vkCmdSetViewport(cmd,  0, 1, &Viewport);
    vkCmdSetScissor(cmd, 0, 1, &Scissor);

    //u32 CurrentFrameIndex = GlobalVulkan._CurrentFrameData % FRAME_OVERLAP;

    VkPipelineLayout PipelineLayout = 
        GlobalVulkan.PipelinesDefinition[PipelineIndex].PipelineLayout;

    GlobalVulkan.CurrentPipelineLayout = PipelineLayout;

    return 0;
}

i32
RenderDrawMeshIndexed(u32 IndicesSize, u32 TotalMeshInstances)
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;

    vkCmdDrawIndexed(cmd,IndicesSize,TotalMeshInstances,0,0,0);

    return 0;
}

i32
RenderPushMeshIndexed(i32 MeshID)
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;

    gpu_heap_block * VertexBlock = FindHeapBlockByID(GlobalVulkan.VertexHeap  , MeshID);
    gpu_heap_block * IndexBlock  = FindHeapBlockByID(GlobalVulkan.IndexHeap   , MeshID);

    VkDeviceSize   VertexOffset = GlobalVulkan.VertexHeap->DeviceBindingOffsetBegin + VertexBlock->Begin;
    VkDeviceSize   IndexOffset  = GlobalVulkan.IndexHeap->DeviceBindingOffsetBegin  + IndexBlock->Begin;

    u32 IndicesSize = IndexBlock->Size / sizeof(u16);

    vkCmdBindVertexBuffers(cmd, 0, 1, &GlobalVulkan.VertexHeap->Buffer, &VertexOffset);
    vkCmdBindIndexBuffer  (cmd, GlobalVulkan.IndexHeap->Buffer, IndexOffset, VK_INDEX_TYPE_UINT16);

    return IndicesSize;
}

i32
RenderPushMeshIndexedAndDraw(u32 TotalMeshInstances, i32 MeshID)
{
    i32 IndicesSize = RenderPushMeshIndexed(MeshID);
    RenderDrawMeshIndexed(TotalMeshInstances, IndicesSize);

    return !(IndicesSize > 0);
}

i32
RenderPushSimulationData(GPUSimulationData * SimData)
{
    void * WriteToAddr;

    u32 CurrentFrame = GlobalVulkan._CurrentFrameData % FRAME_OVERLAP;

    VkDeviceSize DeviceSize = sizeof(GPUSimulationData);

    VkDeviceSize Offset = 
        VH_PaddedUniformBuffer(sizeof(GPUSimulationData)) * CurrentFrame;

    VkMemoryMapFlags Flags = 0; // RESERVED FUTURE USE

    gpu_arena * Arena = GlobalVulkan.SimulationArena;
    //VulkanWriteDataToArena(Arena, (void *)SimData, sizeof(GPUSimulationData));
    VkDeviceMemory DeviceMemory = GetDeviceMemory(Arena->MemoryIndexType);

    VK_CHECK(vkMapMemory(GlobalVulkan.PrimaryDevice,
                         DeviceMemory, 
                         Arena->DeviceBindingOffsetBegin + (u32)Offset, DeviceSize, Flags , &WriteToAddr));

    memcpy(WriteToAddr, (void *)SimData, DeviceSize);

    vkUnmapMemory(GlobalVulkan.PrimaryDevice,DeviceMemory);

    return 0;
}

i32
EndRenderPass()
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;

    if (cmd)
    {
        u32 SwapchainImageIndex = GlobalVulkan.CurrentSwapchainImageIndex;

        vkCmdEndRenderPass(cmd);

#if 1
        VkImageSubresourceRange Range;
        Range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        Range.baseMipLevel   = 0; // uint32_t baseMipLevel;
        Range.levelCount     = 1; // uint32_t levelCount;
        Range.baseArrayLayer = 0; // uint32_t baseArrayLayer;
        Range.layerCount     = 1; // uint32_t layerCount;

        VkImageMemoryBarrier MemoryBarrierPresent;
        MemoryBarrierPresent.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; // sType   sType
        MemoryBarrierPresent.pNext               = 0; // Void * pNext
        MemoryBarrierPresent.srcAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // srcAccessMask   srcAccessMask
        MemoryBarrierPresent.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT; // dstAccessMask   dstAccessMask
        MemoryBarrierPresent.oldLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // oldLayout   oldLayout
        MemoryBarrierPresent.newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // newLayout   newLayout
        MemoryBarrierPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // srcQueueFamilyIndex   srcQueueFamilyIndex
        MemoryBarrierPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // dstQueueFamilyIndex   dstQueueFamilyIndex
        MemoryBarrierPresent.subresourceRange    = Range; // subresourceRange   subresourceRange

        MemoryBarrierPresent.image               = GlobalVulkan.SwapchainImages[SwapchainImageIndex];
        vkCmdPipelineBarrier(cmd, 
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &MemoryBarrierPresent);
#else
        VH_TranstionTo(cmd, 
                GlobalVulkan.SwapchainImages[SwapchainImageIndex], 
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT);

#endif

        VK_CHECK(vkEndCommandBuffer(cmd));

        VkPipelineStageFlags WaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo SubmitInfo     = {};
        SubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;         // VkStructureType   sType;
        SubmitInfo.pNext                = 0;                                     // Void * pNext;
        SubmitInfo.waitSemaphoreCount   = 1;                                     // u32_t   waitSemaphoreCount;
        SubmitInfo.pWaitSemaphores      = &GetCurrentFrame()->ImageAvailableSemaphore; // Typedef * pWaitSemaphores;
        SubmitInfo.pWaitDstStageMask    = &WaitStage;                            // Typedef * pWaitDstStageMask;
        SubmitInfo.commandBufferCount   = 1;                                     // u32_t   commandBufferCount;
        SubmitInfo.pCommandBuffers      = &cmd;                                  // Typedef * pCommandBuffers;
        SubmitInfo.signalSemaphoreCount = 1;                                     // u32_t   signalSemaphoreCount;
        SubmitInfo.pSignalSemaphores    = &GetCurrentFrame()->RenderSemaphore;         // Typedef * pSignalSemaphores;

        VK_CHECK(vkQueueSubmit(GlobalVulkan.GraphicsQueue, 1, &SubmitInfo, GetCurrentFrame()->RenderFence));

        VkPresentInfoKHR PresentInfo;
        PresentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR; // VkStructureType   sType;
        PresentInfo.pNext              = 0; // Void * pNext;
        PresentInfo.waitSemaphoreCount = 1; // u32_t   waitSemaphoreCount;
        PresentInfo.pWaitSemaphores    = &GetCurrentFrame()->RenderSemaphore; // Typedef * pWaitSemaphores;
        PresentInfo.swapchainCount     = 1; // u32_t   swapchainCount;
        PresentInfo.pSwapchains        = &GlobalVulkan.Swapchain; // Typedef * pSwapchains;
        PresentInfo.pImageIndices      = &SwapchainImageIndex; // Typedef * pImageIndices;
        PresentInfo.pResults           = 0; // Typedef * pResults;

        VK_CHECK(vkQueuePresentKHR(GlobalVulkan.GraphicsQueue, &PresentInfo));
    }

    ++GlobalVulkan._CurrentFrameData;

    return 0;
}


i32
RenderEndPassNoPresent()
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;
    //u32 SwapchainImageIndex = GlobalVulkan.CurrentSwapchainImageIndex;

    vkCmdEndRenderPass(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkPipelineStageFlags WaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo SubmitInfo     = {};
    SubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;         // VkStructureType   sType;
    SubmitInfo.pNext                = 0;                                     // Void * pNext;
    SubmitInfo.waitSemaphoreCount   = 0;                                     // u32_t   waitSemaphoreCount;
    SubmitInfo.pWaitSemaphores      = 0;//&GetCurrentFrame()->ImageAvailableSemaphore; // Typedef * pWaitSemaphores;
    SubmitInfo.pWaitDstStageMask    = &WaitStage;                            // Typedef * pWaitDstStageMask;
    SubmitInfo.commandBufferCount   = 1;                                     // u32_t   commandBufferCount;
    SubmitInfo.pCommandBuffers      = &cmd;                                  // Typedef * pCommandBuffers;
    SubmitInfo.signalSemaphoreCount = 0;                                     // u32_t   signalSemaphoreCount;
    SubmitInfo.pSignalSemaphores    = 0;//&GetCurrentFrame()->RenderSemaphore;         // Typedef * pSignalSemaphores;

    VK_CHECK(vkQueueSubmit(GlobalVulkan.GraphicsQueue, 1, &SubmitInfo, GetCurrentFrame()->RenderFence));

    ++GlobalVulkan._CurrentFrameData;

    return 0;
}


void
FreeSwapchain()
{
    u32 SwapchainImageCount = GlobalVulkan.SwapchainImageCount;

    b32 KeepSwapchainAlive = true;
    HierarchyTreeDropBranch(GlobalVulkan.HTree, GlobalVulkan.Swapchain, KeepSwapchainAlive);

    if (SwapchainImageCount > 0)
    {
        for (i32 FrameIndex = 0;
                FrameIndex < FRAME_OVERLAP;
                ++FrameIndex)
        {
            HierarchyTreeDropBranch(GlobalVulkan.HTree, GlobalVulkan.FrameData[FrameIndex].PrimaryCommandBuffer);
        }
#if 0 // I only see format relies on swapchain. Format won't change
        for (i32 FrameIndex = 0;
                FrameIndex < FRAME_OVERLAP;
                ++FrameIndex)
        {
            frame_data * FrameData = GlobalVulkan.FrameData + FrameIndex;
            vkFreeCommandBuffers(GlobalVulkan.PrimaryDevice,FrameData->CommandPool, 1,&FrameData->PrimaryCommandBuffer);
        }

      //
        if ( VK_VALID_HANDLE(GlobalVulkan.RenderPass) )
        {
            vkDestroyRenderPass(GlobalVulkan.PrimaryDevice, GlobalVulkan.RenderPass, 0);
        }

        if ( VK_VALID_HANDLE(GlobalVulkan.RenderPassTransparency) )
        {
            vkDestroyRenderPass(GlobalVulkan.PrimaryDevice, GlobalVulkan.RenderPassTransparency, 0);
        }
#endif

        GlobalVulkan.SwapchainImageCount = 0;
    }

}

i32
VulkanCreateFrameCommandBuffers()
{
    // (fef) for each frame
    for (i32 FrameIndex = 0;
            FrameIndex < FRAME_OVERLAP;
            ++FrameIndex)
    {
        frame_data * FrameData = GlobalVulkan.FrameData + FrameIndex;

        if (VH_CreateCommandBuffers(
                    GlobalVulkan.PrimaryDevice,
                    FrameData->CommandPool,
                    1,
                    &FrameData->PrimaryCommandBuffer)) return 1;

        VULKAN_TREE_APPEND(vkFreeCommandBuffers, (GlobalVulkan.FrameData + FrameIndex)->CommandPool, 
                                                      (GlobalVulkan.FrameData + FrameIndex)->PrimaryCommandBuffer);
    }

    return 0;

}

VkImageCreateInfo
VH_DepthBufferCreateInfo(VkExtent3D Extent)
{
    VkFormat Format = VK_FORMAT_D32_SFLOAT;
    VkImageUsageFlags Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageCreateInfo ImageCreateInfo = VH_CreateImageCreateInfo2D(Extent, Format, Usage);

    return ImageCreateInfo;
}




VkWriteDescriptorSet
VH_WriteDescriptor(u32 BindingSlot,VkDescriptorSet Set,VkDescriptorType DescriptorType, VkDescriptorImageInfo * DescriptorInfo)
{
    VkWriteDescriptorSet WriteDescriptor;

    WriteDescriptor.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; // VkStructureType   sType;
    WriteDescriptor.pNext            = 0; // Void * pNext;
    WriteDescriptor.dstSet           = Set; // VkDescriptorSet   dstSet;
    WriteDescriptor.dstBinding       = BindingSlot; // uint32_t   dstBinding;
    WriteDescriptor.dstArrayElement  = 0; // uint32_t   dstArrayElement;
    WriteDescriptor.descriptorCount  = 1; // uint32_t   descriptorCount;
    WriteDescriptor.descriptorType   = DescriptorType;
    WriteDescriptor.pImageInfo       = DescriptorInfo; // Typedef * pImageInfo;
    WriteDescriptor.pBufferInfo      = 0; // Typedef * pBufferInfo;
    WriteDescriptor.pTexelBufferView = 0; // Typedef * pTexelBufferView;

    return WriteDescriptor;
}

VkWriteDescriptorSet
VH_WriteDescriptor(u32 BindingSlot,VkDescriptorSet Set,VkDescriptorType DescriptorType, VkDescriptorBufferInfo * BufferInfo)
{
    VkWriteDescriptorSet WriteDescriptor;

    WriteDescriptor.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; // VkStructureType   sType;
    WriteDescriptor.pNext            = 0; // Void * pNext;
    WriteDescriptor.dstSet           = Set; // VkDescriptorSet   dstSet;
    WriteDescriptor.dstBinding       = BindingSlot; // uint32_t   dstBinding;
    WriteDescriptor.dstArrayElement  = 0; // uint32_t   dstArrayElement;
    WriteDescriptor.descriptorCount  = 1; // uint32_t   descriptorCount;
    WriteDescriptor.descriptorType   = DescriptorType;
    WriteDescriptor.pImageInfo       = 0; // Typedef * pImageInfo;
    WriteDescriptor.pBufferInfo      = BufferInfo; // Typedef * pBufferInfo;
    WriteDescriptor.pTexelBufferView = 0; // Typedef * pTexelBufferView;

    return WriteDescriptor;
}


vulkan_image *
VH_CreateDepthBuffer(gpu_arena * Arena, VkExtent3D Extent)
{

    Assert(Arena->Type == gpu_arena_type_image);
    Assert(Arena->ImageCount < ArrayCount(Arena->Images));

    vulkan_image VulkanImage = {};

    VkDeviceMemory DeviceMemory = GetDeviceMemory(Arena->MemoryIndexType);
    VkImageCreateInfo ImageCreateInfo = VH_DepthBufferCreateInfo(Extent);
    VulkanImage.Format = ImageCreateInfo.format;       // VkFormat Format;

    if (VK_FAILS(vkCreateImage(Arena->Device,&ImageCreateInfo, 0, &VulkanImage.Image)))
    {
        Logn("Failed to create depth buffer image");
        return 0;
    }

    vkGetImageMemoryRequirements(Arena->Device, VulkanImage.Image, &VulkanImage.MemoryRequirements);
    VkMemoryPropertyFlags PropertyFlags = VK_MEMORY_GPU;

    i32 MemoryTypeIndex = VH_FindSuitableMemoryIndex(Arena->GPU,VulkanImage.MemoryRequirements,PropertyFlags);
    Assert(MemoryTypeIndex == Arena->MemoryIndexType);

    VkDeviceSize BindMemoryOffset = Arena->DeviceBindingOffsetBegin + Arena->CurrentSize;

    if (VK_FAILS(vkBindImageMemory(Arena->Device, VulkanImage.Image,DeviceMemory, BindMemoryOffset)))
    {
        VH_DestroyImage(Arena->Device,&VulkanImage);
        return 0;
    }

    VkImageSubresourceRange SubresourceRange;
    SubresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT; // VkImageAspectFlags   aspectMask;
    SubresourceRange.baseMipLevel   = 0; // u32_t   baseMipLevel;
    SubresourceRange.levelCount     = 1; // u32_t   levelCount;
    SubresourceRange.baseArrayLayer = 0; // u32_t   baseArrayLayer;
    SubresourceRange.layerCount     = 1; // u32_t   layerCount;

    VkImageViewCreateInfo ImageViewCreateInfo = {}; 

    ImageViewCreateInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; // VkStructureType   sType;
    ImageViewCreateInfo.pNext    = 0;                     // Void * pNext;
    ImageViewCreateInfo.flags    = 0;                     // VkImageViewCreateFlags flags;
    ImageViewCreateInfo.image    = VulkanImage.Image;                 // VkImage image;
    ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // VkImageViewType viewType;
    ImageViewCreateInfo.format   = ImageCreateInfo.format;
    //ImageViewCreateInfo.components       = ; // VkComponentMapping   components;
    ImageViewCreateInfo.subresourceRange = SubresourceRange; // VkImageSubresourceRange   subresourceRange;

    if (VK_FAILS(vkCreateImageView( Arena->Device, &ImageViewCreateInfo, 0, &VulkanImage.ImageView )))
    {
        VH_DestroyImage(Arena->Device,&VulkanImage);
        return 0;
    }

    vulkan_image * ResultImage = Arena->Images + Arena->ImageCount++;
    *ResultImage = VulkanImage;

    Arena->CurrentSize += (u32)VulkanImage.MemoryRequirements.size;

    return ResultImage;
}

i32
VH_CreateUnAllocArenaImage(VkPhysicalDevice PhysicalDevice,
                           VkDevice Device,
                           VkImageCreateInfo * CreateInfo, VkImageAspectFlagBits PlaneAspect,
                           VkMemoryPropertyFlags MemoryPropertyFlags,
                           gpu_arena * Arena)
{

    VkMemoryRequirements2 MemoryRequirements = {};
    MemoryRequirements.sType                     = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2; // sType   sType
    MemoryRequirements.pNext                     = 0; // Void * pNext
    MemoryRequirements.memoryRequirements        = {}; // memoryRequirements   memoryRequirements
    
    VkDeviceImageMemoryRequirements ReqImage;
    ReqImage.sType       = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS; // sType   sType
    ReqImage.pNext       = 0; // Void * pNext
    ReqImage.pCreateInfo = CreateInfo; // VkImageCreateInfo * pCreateInfo
    ReqImage.planeAspect = PlaneAspect; // planeAspect   planeAspect

    vkGetDeviceImageMemoryRequirements(GlobalVulkan.PrimaryDevice,&ReqImage,&MemoryRequirements);
    VkMemoryPropertyFlags ImageMemoryPropertyFlags = MemoryPropertyFlags;
    i32 MemoryTypeIndex = VH_FindSuitableMemoryIndex(GlobalVulkan.PrimaryGPU,MemoryRequirements,ImageMemoryPropertyFlags);

    Arena->MemoryIndexType = -1; // MemoryIndexType   MemoryIndexType
    Arena->GPU             = PhysicalDevice; // GPU   GPU
    Arena->Device          = Device; // Device   Device
    Arena->MaxSize         = MemoryRequirements.memoryRequirements.size; // MaxSize   MaxSize
    Arena->CurrentSize     = 0; // CurrentSize   CurrentSize
    Arena->Alignment       = 0; // Alignment   Alignment
    Arena->Type            = gpu_arena_type_image; // ENUM   Type
    Arena->WriteToAddr     = 0; // Void * WriteToAddr
    Arena->Buffer          = VK_NULL_HANDLE; // Buffer   Buffer
    Arena->ImageCount      = 0; // ImageCount   ImageCount
    for (u32 ImageIndex = 0;
                ImageIndex < ArrayCount(Arena->Images);
                ++ImageIndex)
    {
        vulkan_image * Image = Arena->Images + ImageIndex;
        Image->Image                 = VK_NULL_HANDLE; // Image   Image
        Image->ImageView             = VK_NULL_HANDLE; // ImageView   ImageView
        Image->Format                = {}; // Format   Format
        Image->MemoryRequirements    = {}; // MemoryRequirements   MemoryRequirements
    }

    Arena->MemoryIndexType = MemoryTypeIndex;
    Arena->Alignment = (u32)MemoryRequirements.memoryRequirements.alignment;

    return 0;
}

i32 
OnWindowResize(i32 Width,i32 Height)
{
    if (GlobalVulkan.Initialized)
    {
        VulkanWaitForDevices();

        u32 SwapchainImageCount = GlobalVulkan.SwapchainImageCount;

        if (SwapchainImageCount > 0)
        {
            for (i32 FrameIndex = 0;
                    FrameIndex < FRAME_OVERLAP;
                    ++FrameIndex)
            {
                HierarchyTreeDropBranch(GlobalVulkan.HTree, GlobalVulkan.FrameData[FrameIndex].PrimaryCommandBuffer);
            }
        }
        
        if (VulkanCreateSwapChain(Width, Height)) return 1;

        // If window is minimized process is halt
        if (GlobalWindowIsMinimized) return 0;

        if (VulkanCreateFrameCommandBuffers()) return 1;

#if 0 // again do we really need?
        if (VulkanInitDefaultRenderPass()) return 1;
        if (VulkanCreateTransparentRenderPass()) return 1;
#endif

        VH_DestroyImage(GlobalVulkan.PrimaryDepthBufferArena->Device,GlobalVulkan.PrimaryDepthBuffer);
        --GlobalVulkan.PrimaryDepthBufferArena->ImageCount;
        Assert(GlobalVulkan.PrimaryDepthBufferArena->ImageCount == 0);

        VkExtent3D Extent3D = { GlobalVulkan.WindowExtension.width, GlobalVulkan.WindowExtension.height, 1};
        GlobalVulkan.PrimaryDepthBuffer = VH_CreateDepthBuffer(GlobalVulkan.PrimaryDepthBufferArena,Extent3D);
        if (IS_NULL(GlobalVulkan.PrimaryDepthBuffer))
        {
            Logn("Failed to create depth buffer");
            return 1;
        }

        // WEIGHTED
        VkImageUsageFlags UsageFlags = GlobalVulkan.WeightedColorImage->UsageFlags;
        VH_DestroyImage(GlobalVulkan.WeightedColorArena->Device, GlobalVulkan.WeightedColorImage);
        GlobalVulkan.WeightedColorArena->ImageCount -= 1;
        GlobalVulkan.WeightedColorImage = VH_CreateImage(GlobalVulkan.WeightedColorArena, VK_FORMAT_R16G16B16A16_SFLOAT, UsageFlags, Extent3D);
        if (IS_NULL(GlobalVulkan.WeightedColorImage))
        {
            Logn("Failed to create weighted color image");
        }
        SetDebugName(GlobalVulkan.WeightedColorImage->Image, "WeightedColorImage");

        // REVEAL
        UsageFlags = GlobalVulkan.WeightedRevealImage->UsageFlags;
        VH_DestroyImage(GlobalVulkan.WeightedRevealArena->Device, GlobalVulkan.WeightedRevealImage);
        GlobalVulkan.WeightedRevealArena->ImageCount -= 1;
        GlobalVulkan.WeightedRevealImage = VH_CreateImage(GlobalVulkan.WeightedRevealArena, VK_FORMAT_R16_SFLOAT, UsageFlags, Extent3D);
        if (IS_NULL(GlobalVulkan.WeightedRevealImage))
        {
            Logn("Failed to create weighted reveal image");
        }
        SetDebugName(GlobalVulkan.WeightedRevealImage->Image, "WeightedRevealImage");

        if (VulkanInitFramebuffers()) return 1;

        //VulkanReCreatePipelinesOnWindowResize();
    }

    return 0;
}


i32
VulkanCreateLogicaDevice()
{
    vulkan_device_extensions VulkanDeviceExt = GetRequiredDeviceExtensions();
    const char ** DeviceExtensionsRequired = &VulkanDeviceExt.DeviceExtensions[0];
    u32 TotalDeviceExtReq = VulkanDeviceExt.Count;

    r32 QueuePriorities[1] = { 1.0f };
    VkDeviceQueueCreateInfo QueueCreateInfo[3];
    u32 QueuesRequired = 1;

    QueueCreateInfo[0] = {
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     // VkStructureType              sType
      nullptr,                                        // const void                  *pNext
      0,                                              // VkDeviceQueueCreateFlags     flags
      GlobalVulkan.GraphicsQueueFamilyIndex,          // u32_t                     queueFamilyIndex
      ArrayCount(QueuePriorities),                    // u32_t                     queueCount
      &QueuePriorities[0]                             // const float                 *pQueuePriorities
    };

    if (GlobalVulkan.GraphicsQueueFamilyIndex != GlobalVulkan.PresentationQueueFamilyIndex)
    {
        QueueCreateInfo[QueuesRequired] = {
          VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,         // VkStructureType              sType
          nullptr,                                            // const void                  *pNext
          0,                                                  // VkDeviceQueueCreateFlags     flags
          GlobalVulkan.PresentationQueueFamilyIndex,          // u32_t                     queueFamilyIndex
          ArrayCount(QueuePriorities),                        // u32_t                     queueCount
          &QueuePriorities[0]                                 // const float                 *pQueuePriorities
        };
        ++QueuesRequired;
    }

    if (GlobalVulkan.TransferOnlyQueueFamilyIndex != GlobalVulkan.GraphicsQueueFamilyIndex)
    {
        QueueCreateInfo[QueuesRequired] = {
          VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,         // VkStructureType              sType
          nullptr,                                            // const void                  *pNext
          0,                                                  // VkDeviceQueueCreateFlags     flags
          GlobalVulkan.TransferOnlyQueueFamilyIndex,          // u32_t                     queueFamilyIndex
          ArrayCount(QueuePriorities),                        // u32_t                     queueCount
          &QueuePriorities[0]                                 // const float                 *pQueuePriorities
        };
        ++QueuesRequired;
    }

#if 0
    /* IM PASSING physicaldevicefeatures2 struct with all data requires */
    VkPhysicalDeviceFeatures EnabledFeatures = {};
    EnabledFeatures.geometryShader = VK_TRUE;
    EnabledFeatures.samplerAnisotropy = VK_TRUE;
    EnabledFeatures
#endif

    /*
     * VULKAN 1.3 gl_BaseInstance requires extension DrawParameters in shaders
     * Need to enable 11 features
     * https://www.reddit.com/r/vulkan/comments/or8e8u/vkcreateshadermodule_runtime_validation_error/
     * https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#VkPhysicalDeviceShaderDrawParametersFeatures
     */
    VkPhysicalDeviceVulkan11Features PhysicalDevice11Features;
    PhysicalDevice11Features.sType                              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES; // sType   sType
    PhysicalDevice11Features.pNext                              = 0; // Void * pNext
#if 0
    PhysicalDevice11Features.storageBuffer16BitAccess           = ; // storageBuffer16BitAccess   storageBuffer16BitAccess
    PhysicalDevice11Features.uniformAndStorageBuffer16BitAccess = ; // uniformAndStorageBuffer16BitAccess   uniformAndStorageBuffer16BitAccess
    PhysicalDevice11Features.storagePushConstant16              = ; // storagePushConstant16   storagePushConstant16
    PhysicalDevice11Features.storageInputOutput16               = ; // storageInputOutput16   storageInputOutput16
    PhysicalDevice11Features.multiview                          = ; // multiview   multiview
    PhysicalDevice11Features.multiviewGeometryShader            = ; // multiviewGeometryShader   multiviewGeometryShader
    PhysicalDevice11Features.multiviewTessellationShader        = ; // multiviewTessellationShader   multiviewTessellationShader
    PhysicalDevice11Features.variablePointersStorageBuffer      = ; // variablePointersStorageBuffer   variablePointersStorageBuffer
    PhysicalDevice11Features.variablePointers                   = ; // variablePointers   variablePointers
    PhysicalDevice11Features.protectedMemory                    = ; // protectedMemory   protectedMemory
    PhysicalDevice11Features.samplerYcbcrConversion             = ; // samplerYcbcrConversion   samplerYcbcrConversion
#endif
    PhysicalDevice11Features.shaderDrawParameters               = VK_TRUE; // shaderDrawParameters   shaderDrawParameters

    // Request binding descriptor set AFTER command begin
    VkPhysicalDeviceFeatures2    PhysicalDeviceFeatures2 = {};
    PhysicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    VkPhysicalDeviceDescriptorIndexingFeatures IndexingFeatures = {};
    IndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

    PhysicalDeviceFeatures2.pNext = &IndexingFeatures;

    /* ---------------------- ADDITIONAL FEATURES HERE ---------------------- */
    IndexingFeatures.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
    PhysicalDeviceFeatures2.features.geometryShader            = VK_TRUE;
    PhysicalDeviceFeatures2.features.samplerAnisotropy         = VK_TRUE;
    PhysicalDeviceFeatures2.features.independentBlend          = VK_TRUE;
    PhysicalDeviceFeatures2.features.fillModeNonSolid          = VK_TRUE;


    PhysicalDevice11Features.pNext = &PhysicalDeviceFeatures2;

    VkDeviceCreateInfo DeviceCreateInfo = {};
    DeviceCreateInfo.sType                       = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; // VkStructureType   sType;
    DeviceCreateInfo.pNext                       = &PhysicalDevice11Features;                                    // Void * pNext;
    DeviceCreateInfo.flags                       = 0;                                    // VkDeviceCreateFlags   flags;
    DeviceCreateInfo.queueCreateInfoCount        = QueuesRequired;                       // u32_t   queueCreateInfoCount;
    DeviceCreateInfo.pQueueCreateInfos           = &QueueCreateInfo[0];                  // Typedef * pQueueCreateInfos;
    // DEPRECATED
    DeviceCreateInfo.enabledLayerCount           = 0;                                    // u32_t   enabledLayerCount;
    // DEPRECATED
    DeviceCreateInfo.ppEnabledLayerNames         = 0;                                    // Pointer * ppEnabledLayerNames;
    DeviceCreateInfo.enabledExtensionCount       = TotalDeviceExtReq; // u32_t   enabledExtensionCount;
    DeviceCreateInfo.ppEnabledExtensionNames     = &DeviceExtensionsRequired[0];         // Pointer * ppEnabledExtensionNames;
    DeviceCreateInfo.pEnabledFeatures            = 0;//&EnabledFeatures;                                    // Typedef * pEnabledFeatures;

    VK_CHECK(vkCreateDevice(GlobalVulkan.PrimaryGPU,&DeviceCreateInfo,0,&GlobalVulkan.PrimaryDevice));

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkGetDeviceQueue);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyDevice);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDeviceWaitIdle);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateSemaphore);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroySemaphore);

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateFence);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyFence);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkWaitForFences);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkResetFences);

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateSwapchainKHR);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroySwapchainKHR);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkGetSwapchainImagesKHR);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkAcquireNextImageKHR);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkQueuePresentKHR);

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateRenderPass);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyRenderPass);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateImageView);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateImage);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateSampler);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroySampler);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyImageView);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateFramebuffer);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyFramebuffer);

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateShaderModule);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyShaderModule);

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateGraphicsPipelines);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreatePipelineLayout);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyPipeline);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyPipelineLayout);

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateDescriptorSetLayout);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyDescriptorSetLayout);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkAllocateDescriptorSets);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkUpdateDescriptorSets);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateDescriptorPool);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyDescriptorPool);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCmdBindDescriptorSets);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCmdSetViewport);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCmdSetScissor);
    
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkAllocateMemory);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkFreeMemory);

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateBuffer);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyBuffer);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyImage);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkGetBufferMemoryRequirements);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkGetImageMemoryRequirements);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkGetDeviceImageMemoryRequirements);
    

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkMapMemory);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkUnmapMemory);

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkBindBufferMemory);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkBindImageMemory);

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCmdSetDepthTestEnable);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCmdNextSubpass);
    


    // Queue creation
    vkGetDeviceQueue(GlobalVulkan.PrimaryDevice, GlobalVulkan.GraphicsQueueFamilyIndex, 0, &GlobalVulkan.GraphicsQueue);
    vkGetDeviceQueue(GlobalVulkan.PrimaryDevice, GlobalVulkan.PresentationQueueFamilyIndex, 0, &GlobalVulkan.PresentationQueue);
    vkGetDeviceQueue(GlobalVulkan.PrimaryDevice, GlobalVulkan.TransferOnlyQueueFamilyIndex, 0, &GlobalVulkan.TransferOnlyQueue);

    return 0;
}

vk_version
GetVkVersionFromu32(u32 Version)
{
    vk_version Result = {
        VK_VERSION_MAJOR(Version),
        VK_VERSION_MINOR(Version),
        VK_VERSION_PATCH(Version)
    };
    return Result;
}


i32
VulkanGetPhysicalDevice()
{
    vulkan_device_extensions VulkanDeviceExt = GetRequiredDeviceExtensions();
    const char ** DeviceExtensionsRequired = &VulkanDeviceExt.DeviceExtensions[0];
    u32 TotalDeviceExtReq = VulkanDeviceExt.Count;

    u32 TotalPhysicalDevices = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(GlobalVulkan.Instance,&TotalPhysicalDevices,0));

    VkPhysicalDevice PhysicalDevices[12];
    Assert(ArrayCount(PhysicalDevices) >= TotalPhysicalDevices);
    VK_CHECK(vkEnumeratePhysicalDevices(GlobalVulkan.Instance,&TotalPhysicalDevices,&PhysicalDevices[0]));

    VkPhysicalDevice SelectedDeviceHandle = VK_NULL_HANDLE;
    vk_version SelectedVersion = {};
    u32 SelectedMaxImageDimension2D = 0;
    //u32 SelectedQueueFamilyIndex = UINT32_MAX;

    for (u32 i = 0;
         i < TotalPhysicalDevices;
         ++i)
    {
        VkPhysicalDeviceProperties  PhysicalDeviceProperties;
        vkGetPhysicalDeviceProperties(PhysicalDevices[i], &PhysicalDeviceProperties);

        VkPhysicalDeviceFeatures    PhysicalDeviceFeatures;
        vkGetPhysicalDeviceFeatures(PhysicalDevices[i],   &PhysicalDeviceFeatures);

        /* BEGIN Additional device features query */
        VkPhysicalDeviceFeatures2    PhysicalDeviceFeatures2 = {};
        PhysicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        VkPhysicalDeviceDescriptorIndexingFeatures IndexingFeatures = {};
        IndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

        PhysicalDeviceFeatures2.pNext = &IndexingFeatures;

        vkGetPhysicalDeviceFeatures2(PhysicalDevices[i],   &PhysicalDeviceFeatures2);
        /* END Additional device features query */

        b32 SupportsBindingDescriptorDuringCommand = IndexingFeatures.descriptorBindingSampledImageUpdateAfterBind;

        b32 SupportsAnisotropyFilter = (PhysicalDeviceFeatures.samplerAnisotropy == VK_TRUE);
        b32 SupportsGeometryShaders = (PhysicalDeviceFeatures.geometryShader == VK_TRUE);
        b32 SupportIndependentBlend = (PhysicalDeviceFeatures.independentBlend == VK_TRUE);

        b32 AllFeaturesSupported = SupportsAnisotropyFilter &&
                                   SupportsGeometryShaders  &&
                                   SupportsBindingDescriptorDuringCommand &&
                                   SupportIndependentBlend
                                   ;

        vk_version Version = GetVkVersionFromu32(PhysicalDeviceProperties.apiVersion);

        u32 TotalQueueFamilyPropertyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevices[i],&TotalQueueFamilyPropertyCount,0);

        if (TotalQueueFamilyPropertyCount > 0 && AllFeaturesSupported)
        {

            // TODO: Family queues can have bigger internal queues count and 
            //       more flags which might be preferred.
            //       Need to revisit and understand if makes impact
            u32 GraphicsQueueFamilyIndex     = UINT32_MAX;
            u32 PresentationQueueFamilyIndex = UINT32_MAX;
            u32 TransferOnlyQueueFamilyIndex = UINT32_MAX;

            VkQueueFamilyProperties QueueFamiliyProperties[10];
            Assert(ArrayCount(QueueFamiliyProperties) >= TotalQueueFamilyPropertyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevices[i],&TotalQueueFamilyPropertyCount,&QueueFamiliyProperties[0]);

            for (u32 QueueFamilyIndex = 0;
                QueueFamilyIndex < TotalQueueFamilyPropertyCount;
                ++QueueFamilyIndex)
            {
                VkBool32 QueueSupportsPresentation = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevices[i], QueueFamilyIndex, GlobalVulkan.Surface, &QueueSupportsPresentation);
                if (QueueSupportsPresentation)
                {
                    PresentationQueueFamilyIndex = QueueFamilyIndex;
                }
                if (
                        (QueueFamiliyProperties[QueueFamilyIndex].queueCount > 0) &&
                        (QueueFamiliyProperties[QueueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                   )
                {
                    GraphicsQueueFamilyIndex = QueueFamilyIndex;
                    if (QueueSupportsPresentation)
                    {
                        // queue support both - preferred
                        break;
                    }
                    else
                    {
                        // try to find queue with support for both
                    }
                }

            } // for each QueueFamily

            // Dedicated queue to transfer if available
            // We want to pick the family with the lest amount of queue avaiables
            u32 TotalQueueCount = UINT32_MAX;
            for (u32 QueueFamilyIndex = 0;
                QueueFamilyIndex < TotalQueueFamilyPropertyCount;
                ++QueueFamilyIndex)
            {
                if ( 
                        (QueueFamiliyProperties[QueueFamilyIndex].queueCount < TotalQueueCount) &&
                        (QueueFamiliyProperties[QueueFamilyIndex].queueFlags & VK_QUEUE_TRANSFER_BIT)  &&
                        (!(QueueFamiliyProperties[QueueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT))
                   )
                {
                    TransferOnlyQueueFamilyIndex = QueueFamilyIndex;
                    TotalQueueCount = QueueFamiliyProperties[QueueFamilyIndex].queueCount;
                    // don't break try to find family with smaller queue count
                }
            }

            b32 QueueFamilySuffice = (
                    (PresentationQueueFamilyIndex != UINT32_MAX) && 
                    (GraphicsQueueFamilyIndex     != UINT32_MAX)
            );

            u32 TotalDeviceExtProp = 0;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(PhysicalDevices[i], 0, &TotalDeviceExtProp, 0));

            VkExtensionProperties DeviceExtensionProps[256] = {};
            Assert(ArrayCount(DeviceExtensionProps) >= TotalDeviceExtProp);
            VK_CHECK(vkEnumerateDeviceExtensionProperties(PhysicalDevices[i], 0, &TotalDeviceExtProp, &DeviceExtensionProps[0]));

            b32 AllDeviceExtSupported = true;
            for (u32 RequiredDeviceExtPropIndex = 0;
                 RequiredDeviceExtPropIndex < TotalDeviceExtReq;
                 ++RequiredDeviceExtPropIndex)
            {
                u32 DeviceExtPropIndex = 0;
                for (;
                     DeviceExtPropIndex < TotalDeviceExtProp;
                     ++DeviceExtPropIndex)
                {
                    if (strcmp(DeviceExtensionProps[DeviceExtPropIndex].extensionName,
                                DeviceExtensionsRequired[RequiredDeviceExtPropIndex]) == 0)
                    {
                        break;
                    }
                }
                if (DeviceExtPropIndex == TotalDeviceExtProp)
                {
                    AllDeviceExtSupported = false;
                    break;
                }
            }

            if (
                    (AllDeviceExtSupported) &&
                    (QueueFamilySuffice) &&
                    (Version.Major >= SelectedVersion.Major) && 
                    (PhysicalDeviceProperties.limits.maxImageDimension2D >= 4096) &&
                    (PhysicalDeviceProperties.limits.maxImageDimension2D >= SelectedMaxImageDimension2D)
                )
            {
                SelectedMaxImageDimension2D = PhysicalDeviceProperties.limits.maxImageDimension2D;
                SelectedVersion = Version;
                SelectedDeviceHandle = PhysicalDevices[i];
                GlobalVulkan.GraphicsQueueFamilyIndex = GraphicsQueueFamilyIndex;
                GlobalVulkan.PresentationQueueFamilyIndex = PresentationQueueFamilyIndex;
                if (TransferOnlyQueueFamilyIndex == UINT32_MAX)
                {
                    TransferOnlyQueueFamilyIndex = GraphicsQueueFamilyIndex;
                }
                GlobalVulkan.TransferOnlyQueueFamilyIndex = TransferOnlyQueueFamilyIndex;
                // this does not works. driver has x.x.x.x
                //vk_version DriverVersion = GetVkVersionFromu32(PhysicalDeviceProperties.driverVersion);
            }
        } // Has any QueueFamily
    }

    if (SelectedDeviceHandle == VK_NULL_HANDLE)
    {
        Log("No Device is suitable\n");
        return 1;
    }

    GlobalVulkan.PrimaryGPU = SelectedDeviceHandle;

    return 0;
}

b32
VulkanInstanceSupportsSurface(const char * VulkanSurfaceExtensionNameOS)
{
    u32 TotalInstanceExtensionProp = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &TotalInstanceExtensionProp, 0));

    VkExtensionProperties InstanceExtensionProp[20] = {};
    Assert(ArrayCount(InstanceExtensionProp) >= TotalInstanceExtensionProp);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &TotalInstanceExtensionProp, &InstanceExtensionProp[0]));

    b32 SurfaceSupport       = false;
    b32 SurfaceSupportOS     = false;

    for (u32 i = 0;
        i < TotalInstanceExtensionProp;
        ++i)
    {
        if (!SurfaceSupport && 
            (strcmp(InstanceExtensionProp[i].extensionName,VK_KHR_SURFACE_EXTENSION_NAME) == 0))
        {
            SurfaceSupport = true;
        }
        else if (!SurfaceSupportOS
                && (strcmp(InstanceExtensionProp[i].extensionName,VulkanSurfaceExtensionNameOS) == 0))
        {
            SurfaceSupportOS = true;
        }
    }

    return (SurfaceSupport && SurfaceSupportOS);
}

vulkan_create_instance *
VulkanInstanceInfo(memory_arena * Arena, 
                   const char * VkKHROSSurfaceExtensionName,
                   b32 EnableValidationLayer)
{
    vulkan_create_instance * VulkanInstanceInfo = PushStruct(Arena, vulkan_create_instance);
    RtlZeroMemory(VulkanInstanceInfo, sizeof(vulkan_create_instance));

    VkApplicationInfo * AppInfo = &VulkanInstanceInfo->AppInfo;

    AppInfo->sType                 = VK_STRUCTURE_TYPE_APPLICATION_INFO; // VkStructureType   sType;
    AppInfo->pNext                 = 0; // Void * pNext;
    AppInfo->pApplicationName      = (const char *)PushString(Arena,"Render Engine"); // Char_S * pApplicationName;
    AppInfo->applicationVersion    = RENDER_VERSION; // u32_t   applicationVersion;
    AppInfo->pEngineName           = 0;
    AppInfo->engineVersion         = ENGINE_VERSION; // u32_t   engineVersion;
    AppInfo->apiVersion            = VULKAN_API_VERSION; // u32_t   apiVersion;
    
    const char * ExtensionsRequired[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VkKHROSSurfaceExtensionName,
#if DEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    };

    u32 NumberOfExtensions = ArrayCount(ExtensionsRequired);
    VulkanInstanceInfo->NumberOfExtensions = NumberOfExtensions;
    VulkanInstanceInfo->ExtensionsRequired = PushArray(Arena,NumberOfExtensions,char *);

    for (u32 ExtReqIndex = 0;
             ExtReqIndex < NumberOfExtensions;
             ++ExtReqIndex)
    {
        VulkanInstanceInfo->ExtensionsRequired[ExtReqIndex] = (char *)PushString(Arena, ExtensionsRequired[ExtReqIndex]);
    }

    VulkanInstanceInfo->NumberOfInstanceLayers = 0;
    VulkanInstanceInfo->InstanceLayers = 0;

    if (EnableValidationLayer)
    {
        const char * AllInstanceLayers[] = {
            "VK_LAYER_KHRONOS_validation"
        };
        VulkanInstanceInfo->NumberOfInstanceLayers = 1;
        VulkanInstanceInfo->InstanceLayers = PushArray(Arena,VulkanInstanceInfo->NumberOfInstanceLayers,char *);
        for (u32 ExtReqIndex = 0;
                ExtReqIndex < VulkanInstanceInfo->NumberOfInstanceLayers;
                ++ExtReqIndex)
        {
            VulkanInstanceInfo->InstanceLayers[ExtReqIndex] = (char *)PushString(Arena, AllInstanceLayers[ExtReqIndex]);
        }
    }

    VkInstanceCreateInfo * InstanceCreateInfo = &VulkanInstanceInfo->CreateInfo;

    InstanceCreateInfo->sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // VkStructureType   sType;
    InstanceCreateInfo->pNext                   = 0;                                      // Void * pNext;
    InstanceCreateInfo->flags                   = 0;                                      // VkInstanceCreateFlags   flags;
    InstanceCreateInfo->pApplicationInfo        = AppInfo;                               // Typedef * pApplicationInfo;
    InstanceCreateInfo->enabledLayerCount       = VulkanInstanceInfo->NumberOfInstanceLayers;              // u32_t   enabledLayerCount;
    
    if (VulkanInstanceInfo->NumberOfInstanceLayers > 0)
    {
        InstanceCreateInfo->ppEnabledLayerNames = VulkanInstanceInfo->InstanceLayers;
    }
    InstanceCreateInfo->enabledExtensionCount   = NumberOfExtensions;
    InstanceCreateInfo->ppEnabledExtensionNames = VulkanInstanceInfo->ExtensionsRequired;


    return VulkanInstanceInfo;
}

i32
VulkanGetInstance(vulkan_create_instance * InstanceInfo,
                  void ** pfnOSSurface, const char * OSSurfaceFuncName)
{

    u32 TotalInstanceExtensionProp = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &TotalInstanceExtensionProp, 0));

    VkExtensionProperties InstanceExtensionProp[20] = {};
    Assert(ArrayCount(InstanceExtensionProp) >= TotalInstanceExtensionProp);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &TotalInstanceExtensionProp, &InstanceExtensionProp[0]));

    b32 AnyInstanceExtPropNotFound = false;

    for (u32 ExtReqIndex = 1;// SURFACE IS CHECKED WITH SPECIAL FUNCTION
             ExtReqIndex < InstanceInfo->NumberOfExtensions;
             ++ExtReqIndex)
    {
        u32 InstanceExtPropIndex = 0;
        for (InstanceExtPropIndex = 0; 
            InstanceExtPropIndex < TotalInstanceExtensionProp;
            ++InstanceExtPropIndex)
        {
            if ((strcmp(InstanceExtensionProp[InstanceExtPropIndex].extensionName,InstanceInfo->ExtensionsRequired[ExtReqIndex]) == 0))
            {
                break;
            }
        }
        if (InstanceExtPropIndex == TotalInstanceExtensionProp)
        {
            Logn("Instance extension property (%s) not found.", InstanceInfo->ExtensionsRequired[ExtReqIndex]);
            AnyInstanceExtPropNotFound = true;
        }
    }

    Assert(!AnyInstanceExtPropNotFound);
    if (AnyInstanceExtPropNotFound)
    { 
        return 1;
    }

    u32 InstanceLayerCount;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&InstanceLayerCount,0));

    VkLayerProperties LayerProperties[40];
    Assert(ArrayCount(LayerProperties) >= InstanceLayerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&InstanceLayerCount,&LayerProperties[0]));

    for (u32 LayerIndex = 0;
                LayerIndex < InstanceInfo->NumberOfInstanceLayers;
                ++LayerIndex)
    {
        b32 Found  = false;
        for (u32 LayerPropIndex = 0;
                LayerPropIndex < InstanceLayerCount;
                ++LayerPropIndex)
        {
            Logn("%s",LayerProperties[LayerPropIndex].layerName);
            if (strcmp(LayerProperties[LayerPropIndex].layerName,InstanceInfo->InstanceLayers[LayerIndex]) == 0)
            {
                Found = true;
                break;
            }
        }
        if (!Found)
        {
            Log("Requested Instance Layer %s but it was not found\n.",InstanceInfo->InstanceLayers[LayerIndex]);
            return 1;
        }
    }

    VK_CHECK(vkCreateInstance(&InstanceInfo->CreateInfo,0,&GlobalVulkan.Instance));
    VULKAN_TREE_APPEND_DATA(vkDestroyInstance, 0 , GlobalVulkan.Instance, InstanceInfo);

    VkInstance Instance = GlobalVulkan.Instance;

    // Set pointer to OS dependent function to create surface
    // OS .exe is responsible to cast this pointer and use it
    *pfnOSSurface = vkGetInstanceProcAddr(Instance,OSSurfaceFuncName);

    // Now we can get address of instance functions
    VK_INSTANCE_LEVEL_FN(Instance,vkDestroySurfaceKHR);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceSurfaceSupportKHR);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceSurfaceFormatsKHR);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceSurfacePresentModesKHR);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceMemoryProperties);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceFormatProperties);
    

    // physical devices
    VK_INSTANCE_LEVEL_FN(Instance,vkEnumeratePhysicalDevices);
    VK_INSTANCE_LEVEL_FN(Instance,vkEnumerateDeviceExtensionProperties);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceProperties);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceFeatures);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceFeatures2);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceQueueFamilyProperties);
    VK_INSTANCE_LEVEL_FN(Instance,vkCreateDevice);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetDeviceProcAddr);
    VK_INSTANCE_LEVEL_FN(Instance,vkDestroyInstance);
    VK_INSTANCE_LEVEL_FN(Instance,vkCreateCommandPool);
    VK_INSTANCE_LEVEL_FN(Instance,vkDestroyCommandPool);
    VK_INSTANCE_LEVEL_FN(Instance,vkResetCommandPool);
    VK_INSTANCE_LEVEL_FN(Instance,vkResetCommandBuffer);
    VK_INSTANCE_LEVEL_FN(Instance,vkAllocateCommandBuffers);
    VK_INSTANCE_LEVEL_FN(Instance,vkFreeCommandBuffers);
    VK_INSTANCE_LEVEL_FN(Instance,vkBeginCommandBuffer);
    VK_INSTANCE_LEVEL_FN(Instance,vkEndCommandBuffer);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdCopyBuffer);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdCopyBufferToImage);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdPushConstants);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdBeginRenderPass);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdEndRenderPass);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdPipelineBarrier);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdClearColorImage);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdBindPipeline);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdDraw);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdDrawIndexed);
    VK_INSTANCE_LEVEL_FN(Instance,vkQueueSubmit);
    VK_INSTANCE_LEVEL_FN(Instance,vkQueueWaitIdle);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdBindVertexBuffers);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdBindIndexBuffer);

    return 0;
}

VkDescriptorSetLayoutBinding
VH_CreateDescriptorSetLayoutBinding(u32 BindingSlot,VkDescriptorType DescriptorType,VkShaderStageFlags ShaderStageFlags, u32 ArraySize = 1)
{
    VkDescriptorSetLayoutBinding Result;
    Result.binding            = BindingSlot; // uint32_t   binding;
    Result.descriptorType     = DescriptorType; // VkDescriptorType   descriptorType;
    Result.descriptorCount    = ArraySize; // uint32_t   descriptorCount;
    Result.stageFlags         = ShaderStageFlags; // VkShaderStageFlags   stageFlags;
    Result.pImmutableSamplers = 0; // Typedef * pImmutableSamplers;

    return Result;
}



i32
CreateDescriptorSetAttachmentInputs(VkDescriptorSetLayout * SetLayout)
{
    VkDescriptorSetLayoutBinding WeightedColorInputAttachmentBinding = 
        VH_CreateDescriptorSetLayoutBinding(0,
                VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding DescriptorSetLayoutBindings[] = 
    {
        WeightedColorInputAttachmentBinding
    };

    VkDescriptorSetLayoutCreateInfo SetInfo;
    SetInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO; // VkStructureType   sType;
    SetInfo.pNext        = 0; // Void * pNext;
    SetInfo.flags        = 0; // VkDescriptorSetLayoutCreateFlags   flags;
    SetInfo.bindingCount = ArrayCount(DescriptorSetLayoutBindings); // uint32_t   bindingCount;
    SetInfo.pBindings    = &DescriptorSetLayoutBindings[0]; // Typedef * pBindings;

    VK_CHECK(vkCreateDescriptorSetLayout(GlobalVulkan.PrimaryDevice,&SetInfo,0,SetLayout));

    return 0;
}

i32
CreateDescriptorSetLayoutGlobal()
{
    VkDescriptorSetLayoutBinding SimulationBufferBinding = 
        VH_CreateDescriptorSetLayoutBinding(0,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding DescriptorSetLayoutBindings[] = 
    {
        SimulationBufferBinding
    };

    VkDescriptorSetLayoutCreateInfo SetInfo;
    SetInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO; // VkStructureType   sType;
    SetInfo.pNext        = 0; // Void * pNext;
    SetInfo.flags        = 0; // VkDescriptorSetLayoutCreateFlags   flags;
    SetInfo.bindingCount = ArrayCount(DescriptorSetLayoutBindings); // uint32_t   bindingCount;
    SetInfo.pBindings    = &DescriptorSetLayoutBindings[0]; // Typedef * pBindings;

    VK_CHECK(vkCreateDescriptorSetLayout(GlobalVulkan.PrimaryDevice,&SetInfo,0,&GlobalVulkan._GlobalSetLayout));

    return 0;
}

i32
CreateDescriptorSetLayoutObjects()
{
    VkDescriptorSetLayoutBinding LayoutBinding = 
        VH_CreateDescriptorSetLayoutBinding(0,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                VK_SHADER_STAGE_VERTEX_BIT);

    VkDescriptorSetLayoutBinding DescriptorSetLayoutBindings[] = 
    {
        LayoutBinding
    };

    VkDescriptorSetLayoutCreateInfo SetInfo;
    SetInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO; // VkStructureType   sType;
    SetInfo.pNext        = 0; // Void * pNext;
    SetInfo.flags        = 0; // VkDescriptorSetLayoutCreateFlags   flags;
    SetInfo.bindingCount = ArrayCount(DescriptorSetLayoutBindings); // uint32_t   bindingCount;
    SetInfo.pBindings    = &DescriptorSetLayoutBindings[0]; // Typedef * pBindings;

    VK_CHECK(vkCreateDescriptorSetLayout(GlobalVulkan.PrimaryDevice,&SetInfo,nullptr,&GlobalVulkan._ObjectsSetLayout));

    return 0;
}

i32
CreateDescriptorSetTextures()
{
    VkDescriptorSetLayoutBinding SamplerBinding= 
        VH_CreateDescriptorSetLayoutBinding(0,
                VK_DESCRIPTOR_TYPE_SAMPLER,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                1);
    VkDescriptorSetLayoutBinding SampledImageBinding= 
        VH_CreateDescriptorSetLayoutBinding(1,
                VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                10);

    VkDescriptorSetLayoutBinding DescriptorSetLayoutBindings[] = 
    {
        SamplerBinding,
        SampledImageBinding 
    };


    // TODO: how can we avoid allowing rebinding
    // thsi will be solved asap I figure out how to deal
    // with image descriptor at the beginning of the frame
    // before initializing command buffer
    VkDescriptorBindingFlags BindingFlags[] = {
        VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT,
        VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT
        };

#pragma warning( disable : 4127 )
    Assert(ArrayCount(BindingFlags) == ArrayCount(DescriptorSetLayoutBindings));
#pragma warning( default : 4127 )

    VkDescriptorSetLayoutBindingFlagsCreateInfo  BindingFlagsCreateInfo;
    BindingFlagsCreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO; // sType   sType
    BindingFlagsCreateInfo.pNext         = 0; // Void * pNext
    BindingFlagsCreateInfo.bindingCount  = ArrayCount(DescriptorSetLayoutBindings); // bindingCount   bindingCount
    BindingFlagsCreateInfo.pBindingFlags = BindingFlags; // VkDescriptorBindingFlags * pBindingFlags

    VkDescriptorSetLayoutCreateInfo SetInfo;
    SetInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO; // VkStructureType   sType;
    SetInfo.pNext        = 0;//&BindingFlagsCreateInfo; // Void * pNext;
    SetInfo.flags        = 0; // VkDescriptorSetLayoutCreateFlags   flags;
    SetInfo.bindingCount = ArrayCount(DescriptorSetLayoutBindings); // uint32_t   bindingCount;
    SetInfo.pBindings    = &DescriptorSetLayoutBindings[0]; // Typedef * pBindings;

    VK_CHECK(vkCreateDescriptorSetLayout(GlobalVulkan.PrimaryDevice,&SetInfo,nullptr,&GlobalVulkan._DebugTextureSetLayout));

    VH_AllocateDescriptor(&GlobalVulkan._DebugTextureSetLayout,GlobalVulkan._DescriptorPool,&GlobalVulkan._DebugTextureSet);

    return 0;
}





i32
CreatePipelineLayoutTexture(VkPipelineLayout * PipelineLayout)
{
    VkDescriptorSetLayout LayoutSets[] = 
    {
        GlobalVulkan._GlobalSetLayout,
        GlobalVulkan._ObjectsSetLayout,
        GlobalVulkan._DebugTextureSetLayout,
        GlobalVulkan._oit_WeightedColorAttachmentInputsSetLayout,
        GlobalVulkan._oit_WeightedRevealAttachmentInputsSetLayout
    };

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {};
    PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; // VkStructureType sType;

    VkPushConstantRange PushConstant;
    PushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // VkShaderStageFlags   stageFlags;
    PushConstant.offset     = 0; // u32_t   offset;
    PushConstant.size       = sizeof(mesh_push_constant); // u32_t   size;

    PipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    PipelineLayoutCreateInfo.pPushConstantRanges = &PushConstant;

    PipelineLayoutCreateInfo.setLayoutCount = ArrayCount(LayoutSets);
    PipelineLayoutCreateInfo.pSetLayouts = LayoutSets;

    VK_CHECK(vkCreatePipelineLayout(GlobalVulkan.PrimaryDevice, &PipelineLayoutCreateInfo, 0, PipelineLayout));

    return 0;
}

i32
CreateUnAllocArenaBuffer(VkPhysicalDevice PhysicalDevice,
                      VkDevice Device, VkDeviceSize Size, 
                      VkSharingMode SharingMode, VkMemoryPropertyFlags PropertyFlags, VkBufferUsageFlags Usage,
                      gpu_arena * Arena,
                      u32 SharedBufferQueueFamilyIndexCount = 0,
                      u32 * SharedBufferQueueFamilyIndexArray = 0)
{

    VkBufferCreateInfo BufferCreateInfo;

    BufferCreateInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO; // VkStructureType sType;
    BufferCreateInfo.pNext                 = 0;           // Void * pNext;
    BufferCreateInfo.flags                 = 0;           // VkBufferCreateFlags flags;
    BufferCreateInfo.size                  = Size;        // VkDeviceSize size;
    BufferCreateInfo.usage                 = Usage;       // VkBufferUsageFlags usage;
    BufferCreateInfo.sharingMode           = SharingMode; // VkSharingMode sharingMode;

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

    VK_CHECK(vkCreateBuffer(Device, &BufferCreateInfo,0, &Arena->Buffer));

    VkMemoryRequirements MemoryRequirements;
    vkGetBufferMemoryRequirements(Device, Arena->Buffer, &MemoryRequirements);
    i32 MemoryTypeIndex = VH_FindSuitableMemoryIndex(PhysicalDevice,MemoryRequirements,PropertyFlags);

    Assert(MemoryRequirements.size == Size);
    Arena->MemoryIndexType = MemoryTypeIndex;
    Arena->MaxSize         = (u32)MemoryRequirements.size;
    Arena->Alignment       = (u32)MemoryRequirements.alignment;
    Arena->CurrentSize     = 0;
    Arena->Device          = Device; 
    Arena->Type            = gpu_arena_type_buffer;

    return 0;
}

const char * CTableArenaNames[] = {
    "SimulationArena (Global)",
    "Frame Arena 0",
    "Frame Arena 1",
    "Depth buffer",
    "Weight Color",
    "Weight Reveal",
    "Textures",
    "Transfer CPU GPU",
    "Vertex Buffer",
    "Index buffer"
};

internal i32
CreateUnallocatedHeaps(gpu_heap * Heaps, u32 MaxHeapCount, u32 * HeapsCreatedCount, memory_arena * HeapHeadersArena)
{
    u32 HeapCount = 0;

    /* TEXTURE BUFFER */
    // TODO query max dimensions
    VkExtent3D MaxExtent3D = { 1920, 1080, 1};

    // Query default image required memory and allocate
    VkImageUsageFlags UsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB; // VkFormat   format;
    VkImageCreateInfo DefaultTextureInfo = 
        VH_CreateImageCreateInfo2D(MaxExtent3D, Format, UsageFlags);

    u32 TextureArenaSize = Megabytes(128);
    Assert(HeapCount < MaxHeapCount);
    gpu_heap * TexturesHeap = Heaps + HeapCount++;
    if ( InitializeUnallocHeapBuffer(TexturesHeap,HeapHeadersArena, 
                                     GlobalVulkan.PrimaryGPU, GlobalVulkan.PrimaryDevice,
                                     TextureArenaSize,
                                     4096,
                                     &DefaultTextureInfo, VK_IMAGE_ASPECT_COLOR_BIT,
                                     VK_MEMORY_GPU))
    {
        return 1;
    };
    GlobalVulkan.TextureHeap = TexturesHeap;
    TexturesHeap->Owner = &GlobalVulkan.TextureHeap;


    /* VERTEX BUFFER */
    VkDeviceSize VertexBufferSize = Megabytes(64);

    Assert(HeapCount < MaxHeapCount);
    gpu_heap * VertexHeap = Heaps + HeapCount++;
    if ( InitializeUnallocHeapBuffer(VertexHeap, HeapHeadersArena,
                                     GlobalVulkan.PrimaryGPU, GlobalVulkan.PrimaryDevice,
                                     VertexBufferSize,
                                     NextPowerOf2(sizeof(vertex_point) * 256),
                                     VK_SHARING_MODE_EXCLUSIVE,
                                     VK_MEMORY_GPU,
                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT))
    {
        return 1;
    }
    GlobalVulkan.VertexHeap = VertexHeap;
    VertexHeap->Owner= &GlobalVulkan.VertexHeap;

    /* INDEX BUFFER */
    VkDeviceSize IndexBufferSize = Megabytes(16);
    Assert(HeapCount < MaxHeapCount);
    gpu_heap * IndexHeap = Heaps + HeapCount++;
    if ( InitializeUnallocHeapBuffer(IndexHeap, HeapHeadersArena,
                                     GlobalVulkan.PrimaryGPU, GlobalVulkan.PrimaryDevice,
                                     IndexBufferSize,
                                     NextPowerOf2(sizeof(i32) * 256),
                                     VK_SHARING_MODE_EXCLUSIVE,
                                     VK_MEMORY_GPU,
                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT))
    {
        return 1;
    }
    GlobalVulkan.IndexHeap =  IndexHeap;
    IndexHeap->Owner = &GlobalVulkan.IndexHeap;

    *HeapsCreatedCount = HeapCount;

    return 0;
}

i32
CreateUnallocatedMemoryArenas(gpu_arena * Arenas, i32 MaxArenas, u32 * ArenasCreatedCount)
{
    i32 ArenaCount = 0;

    // each frame we switch global uniform buffer
    VkDeviceSize SimulationBufferPaddedSize = FRAME_OVERLAP * VH_PaddedUniformBuffer(sizeof(GPUSimulationData));
    Assert(ArenaCount < MaxArenas);
    gpu_arena * SimulationArena = Arenas + ArenaCount++;
    if (CreateUnAllocArenaBuffer(
                GlobalVulkan.PrimaryGPU,
                GlobalVulkan.PrimaryDevice,
                SimulationBufferPaddedSize,
                VK_SHARING_MODE_EXCLUSIVE,
                VK_MEMORY_CPU_TO_GPU_PREFERRED,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                SimulationArena)
        )
    {
        return 1;
    }
    GlobalVulkan.SimulationArena = SimulationArena;
    SimulationArena->Owner = &GlobalVulkan.SimulationArena;

    // Maximum number of entities rendered
    u32 TotalFrameObjects = MAX_GPU_FRAME_ENTITIES;
    u32 ObjectsSize = sizeof(GPUObjectData) * TotalFrameObjects;

    for (i32 FrameIndex = 0;
            FrameIndex < FRAME_OVERLAP;
            ++FrameIndex)
    {
        frame_data * FrameData = GlobalVulkan.FrameData + FrameIndex;
        Assert(ArenaCount < MaxArenas);
        gpu_arena * FrameArena = Arenas + ArenaCount++;
        if (CreateUnAllocArenaBuffer(
                    GlobalVulkan.PrimaryGPU,
                    GlobalVulkan.PrimaryDevice,
                    ObjectsSize,
                    VK_SHARING_MODE_EXCLUSIVE,
                    VK_MEMORY_CPU_TO_GPU_PREFERRED,
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    FrameArena)
           )
        {
            return 1;
        }
        FrameData->ObjectsArena = FrameArena;
        FrameArena->Owner = &GlobalVulkan.FrameData[FrameIndex].ObjectsArena;
    }

    // TODO query max dimensions
    VkExtent3D MaxExtent3D = { 1920, 1080, 1};
    VkImageCreateInfo DepthBufferImageInfo = VH_DepthBufferCreateInfo(MaxExtent3D);

    Assert(ArenaCount < MaxArenas);
    gpu_arena * PrimDepthBufArena = Arenas + ArenaCount++;
    if (VH_CreateUnAllocArenaImage(GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice,
                                   &DepthBufferImageInfo, VK_IMAGE_ASPECT_DEPTH_BIT,
                                   VK_MEMORY_GPU,
                                   PrimDepthBufArena)
       )
    {
        return 1;
    };
    GlobalVulkan.PrimaryDepthBufferArena = PrimDepthBufArena;
    PrimDepthBufArena->Owner = &GlobalVulkan.PrimaryDepthBufferArena;

    // Query weight color for transparen objects
    {
        VkImageUsageFlags WeightUsageFlags = 
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        VkFormat Format = VK_FORMAT_R16G16B16A16_SFLOAT; // VkFormat   format;
        VkImageCreateInfo DefaultWeightInfo = 
            VH_CreateImageCreateInfo2D(MaxExtent3D, Format, WeightUsageFlags);

        Assert(ArenaCount < MaxArenas);
        gpu_arena * WeightColorArena = Arenas + ArenaCount++;
        if (VH_CreateUnAllocArenaImage(GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice,
                                       &DefaultWeightInfo, VK_IMAGE_ASPECT_COLOR_BIT, 
                                       VK_MEMORY_GPU,
                                       WeightColorArena)
           )
        {
            return 1;
        };
        GlobalVulkan.WeightedColorArena = WeightColorArena;
        WeightColorArena->Owner = &GlobalVulkan.WeightedColorArena;
    }

    // Query weight reveal for transparen objects
    {
        VkImageUsageFlags WeightUsageFlags = 
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        VkFormat Format = VK_FORMAT_R16_SFLOAT; // VkFormat   format;
        VkImageCreateInfo DefaultWeightInfo = 
            VH_CreateImageCreateInfo2D(MaxExtent3D, Format, WeightUsageFlags);

        Assert(ArenaCount < MaxArenas);
        gpu_arena * WeightRevealArena = Arenas + ArenaCount++;
        if (VH_CreateUnAllocArenaImage(GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice,
                                       &DefaultWeightInfo,VK_IMAGE_ASPECT_COLOR_BIT,
                                       VK_MEMORY_GPU, 
                                       WeightRevealArena)
           )
        {
            return 1;
        };
        GlobalVulkan.WeightedRevealArena = WeightRevealArena;
        WeightRevealArena->Owner = &GlobalVulkan.WeightedRevealArena;
    }

#if 0 // heap now
    // Query default image required memory and allocate
    VkImageUsageFlags UsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB; // VkFormat   format;
    VkImageCreateInfo DefaultTextureInfo = 
        VH_CreateImageCreateInfo2D(MaxExtent3D, Format, UsageFlags);

    u32 TextureArenaSize = Megabytes(128);
    Assert(ArenaCount < MaxArenas);
    gpu_arena * TextureArena = Arenas + ArenaCount++;
    if (VH_CreateUnAllocArenaImage(GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice,
                                   &DefaultTextureInfo,VK_IMAGE_ASPECT_COLOR_BIT,
                                   VK_MEMORY_GPU,
                                   TextureArena)
       )
    {
        return 1;
    };
    TextureArena->MaxSize = TextureArenaSize;
    GlobalVulkan.TextureArena = TextureArena;
    TextureArena->Owner = &GlobalVulkan.TextureArena;
#endif


    VkDeviceSize TransferbitBufferSize = Megabytes(16);
    u32 SharedBufferFamilyIndexArray[2] = {
        GlobalVulkan.GraphicsQueueFamilyIndex,
        GlobalVulkan.TransferOnlyQueueFamilyIndex
    };

    Assert(ArenaCount < MaxArenas);
    gpu_arena * TransferBitArena = Arenas + ArenaCount++;
    if (CreateUnAllocArenaBuffer(
                GlobalVulkan.PrimaryGPU,
                GlobalVulkan.PrimaryDevice, 
                TransferbitBufferSize, 
                VK_SHARING_MODE_CONCURRENT,
                VK_MEMORY_CPU_TO_GPU_HOST_VISIBLE,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                TransferBitArena,
                2,&SharedBufferFamilyIndexArray[0])
        )
    {
        return 1;
    }
    GlobalVulkan.TransferBitArena = TransferBitArena;
    TransferBitArena->Owner = &GlobalVulkan.TransferBitArena;

#if 0 // heap now
    VkDeviceSize VertexBufferSize = Megabytes(50);

    Assert(ArenaCount < MaxArenas);
    gpu_arena * VertexArena = Arenas + ArenaCount++;
    if (CreateUnAllocArenaBuffer(
                GlobalVulkan.PrimaryGPU,
                GlobalVulkan.PrimaryDevice, 
                VertexBufferSize, 
                VK_SHARING_MODE_EXCLUSIVE,
                VK_MEMORY_GPU,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VertexArena)
        )
    {
        return 1;
    }
    GlobalVulkan.VertexArena = VertexArena;
    VertexArena->Owner= &GlobalVulkan.VertexArena;

    VkDeviceSize IndexBufferSize = Megabytes(16);
    Assert(ArenaCount < MaxArenas);
    gpu_arena * IndexArena = Arenas + ArenaCount++;
    if (CreateUnAllocArenaBuffer(
                GlobalVulkan.PrimaryGPU,
                GlobalVulkan.PrimaryDevice, 
                IndexBufferSize, 
                VK_SHARING_MODE_EXCLUSIVE,
                VK_MEMORY_GPU,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                IndexArena)) 
    {
        return 1;
    }
    GlobalVulkan.IndexArena =  IndexArena;
    IndexArena->Owner = &GlobalVulkan.IndexArena;
#endif

    *ArenasCreatedCount = (u32)ArenaCount;

    return 0;
}

i32
CommitArenasToMemory(gpu_arena * Arenas, u32 ArenaCount, device_memory_pools * DeviceMemoryPools)
{
    // pre-step to calculate total memory pool size for each the memory type
    for (u32 ArenaUnallocIndex = 0;
                ArenaUnallocIndex < ArenaCount;
                ++ArenaUnallocIndex)
    {
        gpu_arena * Arena = Arenas + ArenaUnallocIndex;
        Assert(VK_VALID_HANDLE(Arena->Device));
        Assert(Arena->MaxSize > 0);
        Assert(Arena->MemoryIndexType >= 0 && Arena->MemoryIndexType <= (i32)VK_MAX_MEMORY_TYPES);
        Assert( Arena->Type == gpu_arena_type_image || 
                (Arena->Type == gpu_arena_type_buffer && VK_VALID_HANDLE(Arena->Buffer))
            );
        Assert(Arena->CurrentSize == 0);
        Logn("GPU Arena required (%i): %llu (Type: %u, Alignment: %i)",ArenaUnallocIndex,Arena->MaxSize, Arena->MemoryIndexType,Arena->Alignment);
        
        i32 MemoryTypeIndex = Arena->MemoryIndexType;
        device_memory_pool * DeviceMemoryPool = DeviceMemoryPools->DeviceMemoryPool + MemoryTypeIndex;
        DeviceMemoryPool->Device                      = Arena->Device; // Device   Device
        DeviceMemoryPool->DeviceMemory                = VK_NULL_HANDLE; // DeviceMemory   DeviceMemory
        DeviceMemoryPool->Size                        += Arena->MaxSize; // Size   Size
    }

    for (   u32 HeapUnallocIndex = 0;
                HeapUnallocIndex < GlobalVulkan.HeapAllocCount;
                ++HeapUnallocIndex)
    {
        gpu_heap * Heap = GlobalVulkan.HeapAlloc + HeapUnallocIndex;
        Assert(VK_VALID_HANDLE(Heap->Device));
        Assert(Heap->MaxSize > 0);
        Assert(Heap->MemoryIndexType >= 0 && Heap->MemoryIndexType <= (i32)VK_MAX_MEMORY_TYPES);
        Assert(  Heap->Type == gpu_arena_type_image || 
                (Heap->Type == gpu_arena_type_buffer && VK_VALID_HANDLE(Heap->Buffer))
            );
        Assert(Heap->BlockCount == 0);
        Logn("GPU Heap required (%i): %llu (Type: %u, Alignment: %i)",HeapUnallocIndex,Heap->MaxSize, Heap->MemoryIndexType,Heap->Alignment);
        
        i32 MemoryTypeIndex = Heap->MemoryIndexType;
        device_memory_pool * DeviceMemoryPool = DeviceMemoryPools->DeviceMemoryPool + MemoryTypeIndex;
        DeviceMemoryPool->Device       = Heap->Device; // Device Device
        DeviceMemoryPool->DeviceMemory = VK_NULL_HANDLE; // DeviceMemory DeviceMemory
        DeviceMemoryPool->Size        += Heap->MaxSize; // Size Size
    }

    for (i32 MemoryTypeIndex = 0;
                MemoryTypeIndex < (i32)VK_MAX_MEMORY_TYPES;
                ++MemoryTypeIndex)
    {
        device_memory_pool * DeviceMemoryPool = DeviceMemoryPools->DeviceMemoryPool + MemoryTypeIndex;
        if (DeviceMemoryPool->Size > 0)
        {
            Logn("Allocating memory pool index %i with size %llu",MemoryTypeIndex, DeviceMemoryPool->Size);
            VkMemoryAllocateInfo AllocateInfo;

            AllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; // VkStructureType   sType;
            AllocateInfo.pNext           = 0;                       // Void * pNext;
            AllocateInfo.allocationSize  = DeviceMemoryPool->Size;             // VkDeviceSize allocationSize;
            AllocateInfo.memoryTypeIndex = MemoryTypeIndex; // u32_t memoryTypeIndex;

            if (VK_FAILS(vkAllocateMemory(DeviceMemoryPool->Device, &AllocateInfo, 0, &DeviceMemoryPool->DeviceMemory)))
            {
                return 1;
            }

            u64 SizeAllocated = 0;
            for (u32 ArenaUnallocIndex = 0;
                     ArenaUnallocIndex < ArenaCount;
                   ++ArenaUnallocIndex)
            {
                gpu_arena * Arena = Arenas + ArenaUnallocIndex;
                if (Arena->MemoryIndexType == MemoryTypeIndex)
                {
                    // buffer can be bind from start
                    // images on the contrary will be binded
                    // as they are created
                    Assert(((SizeAllocated + Arena->Alignment - 1) & ~((u64)Arena->Alignment - (u64)1)) == SizeAllocated);
                    if (Arena->Type == gpu_arena_type_buffer)
                    {
                        Logn("Binding arena %i to pool %i (offset: %llu)",ArenaUnallocIndex,MemoryTypeIndex,SizeAllocated);
                        VK_CHECK(vkBindBufferMemory(
                                    DeviceMemoryPool->Device,
                                    Arena->Buffer,
                                    DeviceMemoryPool->DeviceMemory,
                                    SizeAllocated)
                                );
                    }
                    Arena->DeviceBindingOffsetBegin = SizeAllocated;
                    SizeAllocated += Arena->MaxSize;
                }
            }

            for (   u32 HeapUnallocIndex = 0;
                    HeapUnallocIndex < GlobalVulkan.HeapAllocCount;
                    ++HeapUnallocIndex)
            {
                gpu_heap * Heap = GlobalVulkan.HeapAlloc + HeapUnallocIndex;

                if (Heap->MemoryIndexType == MemoryTypeIndex)
                {
                    // buffer can be bind from start
                    // images on the contrary will be binded
                    // as they are created
                    Assert(((SizeAllocated + Heap->Alignment - 1) & ~((u64)Heap->Alignment - (u64)1)) == SizeAllocated);
                    if (Heap->Type == gpu_arena_type_buffer)
                    {
                        Logn("Binding arena %i to pool %i (offset: %llu)",HeapUnallocIndex,MemoryTypeIndex,SizeAllocated);
                        VK_CHECK(vkBindBufferMemory(
                                    DeviceMemoryPool->Device,
                                    Heap->Buffer,
                                    DeviceMemoryPool->DeviceMemory,
                                    SizeAllocated)
                                );
                    }
                    Heap->DeviceBindingOffsetBegin = SizeAllocated;
                    SizeAllocated += Heap->MaxSize;
                }

            }

            VULKAN_TREE_APPEND(vkDestroyMemoryPoolCustom,GlobalVulkan.DeviceMemoryPoolsLabel , DeviceMemoryPools->DeviceMemoryPool[MemoryTypeIndex].DeviceMemory);
        }
    }

    return 0;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
              VkDebugUtilsMessageTypeFlagsEXT messageType, 
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
              void* pUserData)
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        Logn("Validation layer: %s", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

PFN_COMPARE(VulkanCompareObject)
{
    i32 Result = -1;

    vulkan_node * Node = (vulkan_node *)((tree_node *)A)->Data;

    if (Node->ID == B)
    {
        Result = 0;
    }

    return Result;
}

PFN_PRINT_NODE(VulkanPrintNode)
{
    vulkan_node * VulkanNode = (vulkan_node *)Node->Data;
    vulkan_node * Parent = 0;
    const char * Name = CTableDestructorDebugNames[VulkanNode->DestructorType];
    const char * ParentName  = "";

    if (VulkanNode->DestructorType == vulkan_destructor_type_vkDestroyUnknown)
    {
        Name = (char *)VulkanNode->ID;
    }

    if (Node->Parent)
    {
        Parent = (vulkan_node *)Node->Parent->Data;
        if (Parent->DestructorType == vulkan_destructor_type_vkDestroyUnknown)
        {
            ParentName = (char *)Parent->ID;
        }
        else
        {
            ParentName = CTableDestructorDebugNames[Parent->DestructorType];
        }
    }

    Logn("%*c%s (Parent: %s",GetNodeDepth(Node) * 4, ' ',Name, ParentName);

}



PFN_DELETE_NODE(VulkanOnTreeNodeDelete)
{
    vulkan_node * VulkanObj = (vulkan_node *)Node->Data;
    vulkan_node * Parent = 0;
    if (ParentNode) Parent = (vulkan_node *)ParentNode->Data;

    const char * DebugName = CTableDestructorDebugNames[VulkanObj->DestructorType];

    Logn("Destroying %s", DebugName);

    switch (VulkanObj->DestructorType)
    {
        case vulkan_destructor_type_vkDestroyUnknown:
            {
                // thsi is a label
            }break;
        // CUSTOM DESTRUCTORS
        case vulkan_destructor_type_vkDestroyMemoryPoolCustom:
            {
                VkDeviceMemory * DeviceMemory = (VkDeviceMemory *)VulkanObj->Owner;
                vkFreeMemory(GlobalVulkan.PrimaryDevice, *DeviceMemory, 0);
                *DeviceMemory = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyHeapCustom:
            {
                gpu_heap ** HeapOwnerPtr = (gpu_heap **)VulkanObj->ID;
                gpu_heap  * Heap         = *HeapOwnerPtr;
                if (Heap->Type == gpu_arena_type_buffer)
                {
                    if (VK_VALID_HANDLE(Heap->Buffer))
                    {
                        vkDestroyBuffer(Heap->Device,Heap->Buffer,0);
                    } 
                }
                else
                {
                    u32 SanityCount = 0;
                    gpu_heap_block * LastBlock = 0;
                    for (gpu_heap_block * Block = Heap->Blocks;
                                          Block;
                                          Block = Block->Next)
                    {
                        vulkan_image * Image = &Block->Image;
                        VH_DestroyImage(Heap->Device,Image);
                        Image->Format    = {};
                        Image->ImageView = VK_NULL_HANDLE;
                        Image->Image     = VK_NULL_HANDLE;
                        LastBlock = Block;
                        ++SanityCount;
                    }
                    LastBlock->Next  = Heap->FreeBlocks;
                    Heap->FreeBlocks = LastBlock;

                    Assert(SanityCount == Heap->BlockCount);
                    Heap->BlockCount = 0;
                }
            } break;
        case vulkan_destructor_type_vkDestroyArenaCustom:
            {
                gpu_arena ** ArenaOwnerPtr = (gpu_arena **)VulkanObj->ID;
                gpu_arena * Arena = *ArenaOwnerPtr;
                if (Arena->Type == gpu_arena_type_buffer)
                {
                    if (VK_VALID_HANDLE(Arena->Buffer))
                    {
                        vkDestroyBuffer(Arena->Device,Arena->Buffer,0);
                    } 
                }
                else
                {
                    Logn("Deallocating Images %i", Arena->ImageCount);
                    for (u32 ImageIndex = 0;
                            ImageIndex < Arena->ImageCount;
                            ++ImageIndex)
                    {
                        vulkan_image * Image = Arena->Images + ImageIndex;
                        VH_DestroyImage(Arena->Device,Image);
                        Image->Format = {};
                        Image->ImageView = VK_NULL_HANDLE;
                        Image->Image = VK_NULL_HANDLE;
                    }
                }
                Arena->MemoryIndexType = -1; // MemoryIndexType   MemoryIndexType
                Arena->GPU             = VK_NULL_HANDLE; // GPU   GPU
                Arena->Device          = VK_NULL_HANDLE; // Device   Device
                Arena->MaxSize         = 0; // MaxSize   MaxSize
                Arena->CurrentSize     = 0; // CurrentSize   CurrentSize
                Arena->Buffer          = VK_NULL_HANDLE; // Buffer   Buffer
                Arena->WriteToAddr     = 0; // Void * WriteToAdd
                *ArenaOwnerPtr = 0;
            } break;


            // GENERIC DESTRUCTOR
        case vulkan_destructor_type_vkFreeCommandBuffers:
            {
                VkCommandPool CommandPool = (VkCommandPool)Parent->ID;
                VkCommandBuffer * CommandBuffer = (VkCommandBuffer *)VulkanObj->Owner;
                vkFreeCommandBuffers(GlobalVulkan.PrimaryDevice,CommandPool, 1,CommandBuffer);
                *CommandBuffer = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyDebugUtilsMessengerEXT:
            {
                VkInstance instance = GlobalVulkan.Instance;
                Assert(VK_VALID_HANDLE(instance));
                VkDebugUtilsMessengerEXT * messenger = (VkDebugUtilsMessengerEXT *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*messenger));
                vkDestroyDebugUtilsMessengerEXT(instance, *messenger,0);
                *messenger = VK_NULL_HANDLE;
            }break;
        case vulkan_destructor_type_vkDestroySurfaceKHR:
            {
                VkInstance instance = GlobalVulkan.Instance;
                Assert(VK_VALID_HANDLE(instance));
                VkSurfaceKHR * surface = (VkSurfaceKHR *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*surface));
                vkDestroySurfaceKHR(instance,*surface,0);
                *surface = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroySwapchainKHR:
            {
                VkDevice device = GlobalVulkan.PrimaryDevice;
                Assert(VK_VALID_HANDLE(device));
                VkSwapchainKHR * swapchain = (VkSwapchainKHR *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*swapchain));
                vkDestroySwapchainKHR(device,*swapchain,0);
                *swapchain = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyInstance:
            {
                VkInstance * instance = (VkInstance *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(instance));
                vkDestroyInstance(*instance,0);
                *instance = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyCommandPool:
            {
                VkDevice device = GlobalVulkan.PrimaryDevice;
                Assert(VK_VALID_HANDLE(device));
                VkCommandPool * commandPool = (VkCommandPool *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*commandPool));
                vkDestroyCommandPool(device,*commandPool,0);
                *commandPool = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyShaderModule:
            {
                VkDevice device = GlobalVulkan.PrimaryDevice;
                Assert(VK_VALID_HANDLE(device));
                VkShaderModule * shaderModule = (VkShaderModule *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*shaderModule));
                vkDestroyShaderModule(device,*shaderModule,0);
                *shaderModule = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyDevice:
            {
                VkDevice * device = (VkDevice *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*device));
                vkDestroyDevice(*device,0);
                *device = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyFence:
            {
                VkDevice device = GlobalVulkan.PrimaryDevice;
                Assert(VK_VALID_HANDLE(device));
                VkFence * fence = (VkFence *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*fence));
                vkDestroyFence(device,*fence,0);
                *fence = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroySemaphore:
            {
                VkDevice device = GlobalVulkan.PrimaryDevice;
                Assert(VK_VALID_HANDLE(device));
                VkSemaphore * semaphore = (VkSemaphore *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*semaphore));
                vkDestroySemaphore(device,*semaphore,0);
                *semaphore = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyRenderPass:
            {
                VkDevice device = GlobalVulkan.PrimaryDevice;
                Assert(VK_VALID_HANDLE(device));
                VkRenderPass * renderPass = (VkRenderPass *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*renderPass));
                vkDestroyRenderPass(device,*renderPass,0);
                *renderPass = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroySampler:
            {
                VkDevice device = GlobalVulkan.PrimaryDevice;
                Assert(VK_VALID_HANDLE(device));
                VkSampler * sampler = (VkSampler *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*sampler));
                vkDestroySampler(device,*sampler,0);
                *sampler = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyImageView:
            {
                VkImageView * imageView = (VkImageView *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*imageView));
                vkDestroyImageView(GlobalVulkan.PrimaryDevice,*imageView,0);
                *imageView = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyFramebuffer:
            {
                VkDevice device = GlobalVulkan.PrimaryDevice;
                Assert(VK_VALID_HANDLE(device));
                VkFramebuffer * framebuffer = (VkFramebuffer *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*framebuffer));
                vkDestroyFramebuffer(device,*framebuffer,0);
                *framebuffer = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyPipeline:
            {
                VkDevice device = GlobalVulkan.PrimaryDevice;
                Assert(VK_VALID_HANDLE(device));
                VkPipeline * pipeline = (VkPipeline *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*pipeline));
                vkDestroyPipeline(device,*pipeline,0);
                *pipeline = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyPipelineLayout:
            {
                VkDevice device = GlobalVulkan.PrimaryDevice;
                Assert(VK_VALID_HANDLE(device));
                VkPipelineLayout * pipelineLayout = (VkPipelineLayout *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*pipelineLayout));
                vkDestroyPipelineLayout(device,*pipelineLayout,0);
                *pipelineLayout = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyDescriptorSetLayout:
            {
                VkDevice device = GlobalVulkan.PrimaryDevice;
                Assert(VK_VALID_HANDLE(device));
                VkDescriptorSetLayout * descriptorSetLayout = (VkDescriptorSetLayout *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*descriptorSetLayout));
                vkDestroyDescriptorSetLayout(device,*descriptorSetLayout,0);
                *descriptorSetLayout = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyDescriptorPool:
            {
                VkDevice device = GlobalVulkan.PrimaryDevice;
                Assert(VK_VALID_HANDLE(device));
                VkDescriptorPool * descriptorPool = (VkDescriptorPool *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*descriptorPool));
                vkDestroyDescriptorPool(device,*descriptorPool,0);
                *descriptorPool = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyBuffer:
            {
                VkDevice device = GlobalVulkan.PrimaryDevice;
                Assert(VK_VALID_HANDLE(device));
                VkBuffer * buffer = (VkBuffer *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*buffer));
                vkDestroyBuffer(device,*buffer,0);
                *buffer = VK_NULL_HANDLE;
            } break;
        case vulkan_destructor_type_vkDestroyImage:
            {
                VkDevice device = GlobalVulkan.PrimaryDevice;
                Assert(VK_VALID_HANDLE(device));
                VkImage * image = (VkImage *)VulkanObj->Owner;
                Assert(VK_VALID_HANDLE(*image));
                //Logn("Image: %p", *image);
                vkDestroyImage(device,*image,0);
                *image = VK_NULL_HANDLE;
            } break;
    }
}


// ivp -- initialize vulkan procedure
i32
InitializeVulkan(i32 Width, i32 Height, 
                 graphics_platform_window PlatformWindow,
                 b32 EnableValidationLayer,
                 PFN_vkGetInstanceProcAddr GetInstanceProcAddr)
{

#pragma warning( disable : 4127 )
    Assert(ArrayCount(CTableDestructorDebugNames) == vulkan_destructortype_LAST);
#pragma warning( default : 4127 )

    InitializeVulkanStruct(&GlobalVulkan);

    // Memory leak fix this
    // get memory from os
    // or clean up on exit
    void * Addr = malloc(Megabytes(10));

    GlobalVulkan.InitArena.Base = (u8 *)Addr;
    GlobalVulkan.InitArena.MaxSize = Megabytes(5);
    GlobalVulkan.InitArena.CurrentSize= 0;

    GlobalVulkan.HTree = CreateHierarchyTree(&GlobalVulkan.InitArena, Megabytes(1), VulkanCompareObject, VulkanOnTreeNodeDelete, VulkanPrintNode);

    vulkan_create_instance * InstanceInfo = 
        VulkanInstanceInfo(&GlobalVulkan.InitArena, PlatformWindow.VkKHROSSurfaceExtensionName,EnableValidationLayer);

    // during creation of vulkan instance
    // base on the OS surface extension name
    // get a void pointer to function which
    // we give back to the OS to cast and invoke
    void * pfnOSSurface = 0;

    vkGetInstanceProcAddr = GetInstanceProcAddr;

    if (!vkGetInstanceProcAddr)
    {
        Log("Global Instance Proc address Function is not valid\n");
        return 1;
    }

    // VULKAN INSTANCE FUNCTIONS
    VK_GLOBAL_LEVEL_FN(vkCreateInstance);
    VK_GLOBAL_LEVEL_FN(vkEnumerateInstanceExtensionProperties);
    VK_GLOBAL_LEVEL_FN(vkEnumerateInstanceLayerProperties);


    if (!VulkanInstanceSupportsSurface(PlatformWindow.OSSurfaceFuncName))
    {
        Logn("Surface (%s) not supported",PlatformWindow.OSSurfaceFuncName);
        //return 1;
    }

    if (VulkanGetInstance(InstanceInfo, &pfnOSSurface,PlatformWindow.OSSurfaceFuncName)) 
    {
        return 1;
    }

#if DEBUG
    VK_INSTANCE_LEVEL_FN(GlobalVulkan.Instance,vkCreateDebugUtilsMessengerEXT);
    VK_INSTANCE_LEVEL_FN(GlobalVulkan.Instance,vkDestroyDebugUtilsMessengerEXT);
    VK_INSTANCE_LEVEL_FN(GlobalVulkan.Instance,vkSetDebugUtilsObjectNameEXT);

    VkDebugUtilsMessengerCreateInfoEXT DebugUtilMsgCreateInfo = {};
    
    DebugUtilMsgCreateInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT; // sType   sType
    DebugUtilMsgCreateInfo.pNext           = 0; // Void * pNext
    DebugUtilMsgCreateInfo.flags           = 0; // flags   flags
    DebugUtilMsgCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT; // messageSeverity   messageSeverity
    DebugUtilMsgCreateInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT  | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT; // messageType   messageType
    DebugUtilMsgCreateInfo.pfnUserCallback = VulkanDebugMessageCallback; // pfnUserCallback   pfnUserCallback
    DebugUtilMsgCreateInfo.pUserData       = 0; // Void * pUserData

    // you can have multiple callbacks for different messages
    vkCreateDebugUtilsMessengerEXT(GlobalVulkan.Instance , &DebugUtilMsgCreateInfo, 0, &GlobalVulkan.DefaultDebugCb);
    VULKAN_TREE_APPEND(vkDestroyDebugUtilsMessengerEXT, GlobalVulkan.Instance,GlobalVulkan.DefaultDebugCb);
#endif

    if (PlatformWindow.pfnVulkanCreateSurface(PlatformWindow.SurfaceData,pfnOSSurface, GlobalVulkan.Instance, &GlobalVulkan.Surface)) return 1;
    VULKAN_TREE_APPEND(vkDestroySurfaceKHR, GlobalVulkan.Instance,GlobalVulkan.Surface);

    if (VulkanGetPhysicalDevice()) return 1;
    if (VulkanCreateLogicaDevice()) return 1;
    VULKAN_TREE_APPEND(vkDestroyDevice, GlobalVulkan.Instance,GlobalVulkan.PrimaryDevice);


    if (VulkanCreateSwapChain(Width, Height)) return 1;

    CreateUnallocatedMemoryArenas(GlobalVulkan.MemoryArenas,
                                  ArrayCount(GlobalVulkan.MemoryArenas),
                                  &GlobalVulkan.MemoryArenaCount);

    for (u32 i = 0; i < GlobalVulkan.MemoryArenaCount;++i)
    {
        gpu_arena * GPUArena = GlobalVulkan.MemoryArenas + i;
        VULKAN_TREE_APPEND_WITH_ID(vkDestroyArenaCustom, GlobalVulkan.PrimaryDevice,GPUArena->Owner, GlobalVulkan.MemoryArenas + i);
    }

    if (CreateUnallocatedHeaps(GlobalVulkan.HeapAlloc, 
                               ArrayCount(GlobalVulkan.HeapAlloc), 
                               &GlobalVulkan.HeapAllocCount, 
                               &GlobalVulkan.InitArena))
    {
        Logn("Unable to allocate reserve space for gpu heap allocators");
        return 1;
    }

    for (u32 i = 0; i < GlobalVulkan.HeapAllocCount;++i)
    {
        gpu_heap * GPUHeap = GlobalVulkan.HeapAlloc + i;
        VULKAN_TREE_APPEND_WITH_ID(vkDestroyHeapCustom, GlobalVulkan.PrimaryDevice,GPUHeap->Owner, GlobalVulkan.HeapAlloc + i);
    }

    VULKAN_TREE_APPEND(vkDestroyUnknown,GlobalVulkan.PrimaryDevice,GlobalVulkan.MemoryArenasLabel);
    VULKAN_TREE_APPEND(vkDestroyUnknown,GlobalVulkan.PrimaryDevice,GlobalVulkan.DeviceMemoryPoolsLabel);
    VULKAN_TREE_APPEND(vkDestroyUnknown,GlobalVulkan.PrimaryDevice,GlobalVulkan.MemoryHeapLabel);

    CommitArenasToMemory(&GlobalVulkan.MemoryArenas[0],
                         GlobalVulkan.MemoryArenaCount,
                         &GlobalVulkan.DeviceMemoryPools);

    for (u32 i = 0; i < GlobalVulkan.MemoryArenaCount;++i)
    {
        gpu_arena * GPUArena = GlobalVulkan.MemoryArenas + i;
        Logn("Arena %-25s(%-2i): %-2i (Memory Index) %-5i (Alignment) %-8i (MaxSize KB)",CTableArenaNames[i],i,GPUArena->MemoryIndexType,GPUArena->Alignment, (u32)(GPUArena->MaxSize / 1024));
    }

    VULKAN_TREE_APPEND(vkDestroyUnknown, GlobalVulkan.PrimaryDevice,GlobalVulkan.DescriptorSetLayoutLabel);


    memory_arena * HeapHashTableArena = AllocateSubArena(&GlobalVulkan.InitArena, Megabytes(1));
    InitializeVulanHashTable(HeapHashTableArena);

    VkDescriptorPoolSize DescriptorPoolSizes[] =
	{
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2 }
	};

	VkDescriptorPoolCreateInfo PoolInfo = {};
	PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	PoolInfo.flags = 0;
	PoolInfo.maxSets = 50;
	PoolInfo.poolSizeCount = ArrayCount(DescriptorPoolSizes);
	PoolInfo.pPoolSizes = DescriptorPoolSizes;

	vkCreateDescriptorPool(GlobalVulkan.PrimaryDevice, &PoolInfo, nullptr, &GlobalVulkan._DescriptorPool);
    SetDebugName(GlobalVulkan._DescriptorPool,"_DescriptorPool");
    VULKAN_TREE_APPEND(vkDestroyDescriptorPool, GlobalVulkan.DescriptorSetLayoutLabel, GlobalVulkan._DescriptorPool);

    /* INIT DESCRIPTOR SETS */
    /* Level 1 */ CreateDescriptorSetLayoutGlobal();
    VULKAN_TREE_APPEND(vkDestroyDescriptorSetLayout, GlobalVulkan.DescriptorSetLayoutLabel, GlobalVulkan._GlobalSetLayout); 
    SetDebugName(GlobalVulkan._GlobalSetLayout,"_GlobalSetLayout");
    /* Level 2 */ CreateDescriptorSetLayoutObjects();
    VULKAN_TREE_APPEND(vkDestroyDescriptorSetLayout, GlobalVulkan.DescriptorSetLayoutLabel, GlobalVulkan._ObjectsSetLayout); 
    SetDebugName(GlobalVulkan._ObjectsSetLayout,"_ObjectsSetLayout");
    /* Level 3 */ CreateDescriptorSetTextures();
    VULKAN_TREE_APPEND(vkDestroyDescriptorSetLayout, GlobalVulkan.DescriptorSetLayoutLabel, GlobalVulkan._DebugTextureSetLayout); 
    SetDebugName(GlobalVulkan._DebugTextureSetLayout,"_DebugTextureSetLayout");
#if 1
    /* Level 4 */ CreateDescriptorSetAttachmentInputs(&GlobalVulkan._oit_WeightedColorAttachmentInputsSetLayout);
    VULKAN_TREE_APPEND(vkDestroyDescriptorSetLayout, GlobalVulkan.DescriptorSetLayoutLabel, GlobalVulkan._oit_WeightedColorAttachmentInputsSetLayout); 
    SetDebugName(GlobalVulkan._oit_WeightedColorAttachmentInputsSetLayout,"_oit_WeightedColorAttachmentInputsSetLayout");

    /* Level 5 */ CreateDescriptorSetAttachmentInputs(&GlobalVulkan._oit_WeightedRevealAttachmentInputsSetLayout);
    VULKAN_TREE_APPEND(vkDestroyDescriptorSetLayout, GlobalVulkan.DescriptorSetLayoutLabel, GlobalVulkan._oit_WeightedRevealAttachmentInputsSetLayout); 
    SetDebugName(GlobalVulkan._oit_WeightedRevealAttachmentInputsSetLayout,"_oit_WeightedRevealAttachmentInputsSetLayout");
#endif

    VULKAN_TREE_APPEND(vkDestroyUnknown,GlobalVulkan.PrimaryDevice,GlobalVulkan.FrameDataLabel);
    // (fef) for each frame
    for (i32 FrameIndex = 0;
            FrameIndex < FRAME_OVERLAP;
            ++FrameIndex)
    {
        frame_data * FrameData = GlobalVulkan.FrameData + FrameIndex;

        VkFenceCreateInfo FenceCreateInfo;

        FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO; // VkStructureType   sType;
        FenceCreateInfo.pNext = 0;                                   // Void * pNext;
        FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;        // VkFenceCreateFlags   flags;

        VK_CHECK(vkCreateFence(GlobalVulkan.PrimaryDevice, &FenceCreateInfo, 0,&FrameData->RenderFence));

        VkSemaphoreCreateInfo SemaphoreCreateInfo = {
            VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            0,
            0
        };

        VK_CHECK(vkCreateSemaphore(GlobalVulkan.PrimaryDevice, &SemaphoreCreateInfo, 0, &FrameData->ImageAvailableSemaphore));
        VK_CHECK(vkCreateSemaphore(GlobalVulkan.PrimaryDevice, &SemaphoreCreateInfo, 0, &FrameData->RenderSemaphore));

        if (VH_CreateCommandPool(
                    GlobalVulkan.PrimaryDevice,
                    GlobalVulkan.PresentationQueueFamilyIndex,
                    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                    &FrameData->CommandPool
                    )) return 1;

#if 0 // Custom function to re-use during swapchain recreation
        if (VH_CreateCommandBuffers(
                    GlobalVulkan.PrimaryDevice,
                    FrameData->CommandPool,
                    1,
                    &FrameData->PrimaryCommandBuffer)) return 1;
#endif

        // from pool, this layout, out ID
        VH_AllocateDescriptor(&GlobalVulkan._GlobalSetLayout, GlobalVulkan._DescriptorPool, &FrameData->GlobalDescriptor);

        VkDescriptorBufferInfo BufferInfo;
        BufferInfo.buffer = GlobalVulkan.SimulationArena->Buffer; // VkBuffer   buffer;
        BufferInfo.offset = 0;
        BufferInfo.range  = sizeof(GPUSimulationData); // VkDeviceSize   range;

        VkWriteDescriptorSet SimulationWriteSet =
            VH_WriteDescriptor(0,FrameData->GlobalDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, &BufferInfo);

        VH_AllocateDescriptor(&GlobalVulkan._ObjectsSetLayout, GlobalVulkan._DescriptorPool, &FrameData->ObjectsDescriptor);

        VkDescriptorBufferInfo BufferInfoObjects;
        BufferInfoObjects.buffer = FrameData->ObjectsArena->Buffer; // VkBuffer   buffer;
        BufferInfoObjects.offset = 0;
        BufferInfoObjects.range  = FrameData->ObjectsArena->MaxSize; // VkDeviceSize   range;

        VkWriteDescriptorSet ObjectsWriteSet =
            VH_WriteDescriptor(0,FrameData->ObjectsDescriptor, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &BufferInfoObjects);

        VkWriteDescriptorSet WriteSets[] = 
        {
            SimulationWriteSet,
            ObjectsWriteSet
        };

        vkUpdateDescriptorSets(GlobalVulkan.PrimaryDevice, ArrayCount(WriteSets), WriteSets, 0, nullptr);
    }

    for (i32 FrameIndex = 0;
            FrameIndex < FRAME_OVERLAP;
            ++FrameIndex)
    {
        VULKAN_TREE_APPEND(vkDestroyFence, GlobalVulkan.FrameDataLabel, (GlobalVulkan.FrameData + FrameIndex)->RenderFence);
        VULKAN_TREE_APPEND(vkDestroySemaphore, GlobalVulkan.FrameDataLabel, (GlobalVulkan.FrameData + FrameIndex)->ImageAvailableSemaphore);
        VULKAN_TREE_APPEND(vkDestroySemaphore, GlobalVulkan.FrameDataLabel, (GlobalVulkan.FrameData + FrameIndex)->RenderSemaphore);
        VULKAN_TREE_APPEND(vkDestroyCommandPool, GlobalVulkan.FrameDataLabel, (GlobalVulkan.FrameData + FrameIndex)->CommandPool);
    }

    // Separate as we need to recreate them on window resizing. After creating Node for command pool
    VulkanCreateFrameCommandBuffers();


    // INIT PIPELINES LAYOUT
    CreatePipelineLayoutTexture(GlobalVulkan.PipelineLayout + 0);
    SetDebugName(GlobalVulkan.PipelineLayout[0], "PipelineLayout Texture");
    VULKAN_TREE_APPEND(vkDestroyPipelineLayout, GlobalVulkan.PrimaryDevice, GlobalVulkan.PipelineLayout[0]);

    VULKAN_TREE_APPEND(vkDestroyUnknown, GlobalVulkan.PrimaryDevice, GlobalVulkan.StagingBufferLabel);
    // Staging buffer commands
    if (VH_CreateCommandPool(
                GlobalVulkan.PrimaryDevice,
                GlobalVulkan.TransferOnlyQueueFamilyIndex,
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                &GlobalVulkan.CommandPoolTransferBit
                )) return 1;

    VULKAN_TREE_APPEND(vkDestroyCommandPool, GlobalVulkan.StagingBufferLabel, GlobalVulkan.CommandPoolTransferBit);

    if (VH_CreateCommandBuffers(
                GlobalVulkan.PrimaryDevice,
                GlobalVulkan.CommandPoolTransferBit,
                1,
                &GlobalVulkan.TransferBitCommandBuffer)) return 1;


    VkExtent3D Extent3D = { GlobalVulkan.WindowExtension.width, GlobalVulkan.WindowExtension.height, 1};
    GlobalVulkan.PrimaryDepthBuffer = VH_CreateDepthBuffer(GlobalVulkan.PrimaryDepthBufferArena,Extent3D);
    if (IS_NULL(GlobalVulkan.PrimaryDepthBuffer))
    {
        Logn("Failed to create depth buffer");
        return 1;
    }
    SetDebugName(GlobalVulkan.PrimaryDepthBuffer->Image, "PrimaryDepthBuffer");

    VkImageUsageFlags WeightUsageFlags = 
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    GlobalVulkan.WeightedColorImage = VH_CreateImage(GlobalVulkan.WeightedColorArena, 
                                                     VK_FORMAT_R16G16B16A16_SFLOAT, 
                                                     WeightUsageFlags, 
                                                     Extent3D);
    if (IS_NULL(GlobalVulkan.WeightedColorImage))
    {
        Logn("Failed to create weighted color image");
    }
    SetDebugName(GlobalVulkan.WeightedColorImage->Image, "WeightedColorImage");


    GlobalVulkan.WeightedRevealImage = VH_CreateImage(GlobalVulkan.WeightedRevealArena, 
                                                     VK_FORMAT_R16_SFLOAT, 
                                                     WeightUsageFlags, 
                                                     Extent3D);
    if (IS_NULL(GlobalVulkan.WeightedRevealImage))
    {
        Logn("Failed to create weighted reveal image");
    }
    SetDebugName(GlobalVulkan.WeightedRevealImage->Image, "WeightedRevealImage");

    VkImageSubresourceRange Range;
    Range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    Range.baseMipLevel   = 0; // uint32_t baseMipLevel;
    Range.levelCount     = 1; // uint32_t levelCount;
    Range.baseArrayLayer = 0; // uint32_t baseArrayLayer;
    Range.layerCount     = 1; // uint32_t layerCount;

    VkImageMemoryBarrier MemoryBarrierSetColorAttachment;
    MemoryBarrierSetColorAttachment.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; // sType   sType
    MemoryBarrierSetColorAttachment.pNext               = 0; // Void * pNext
    MemoryBarrierSetColorAttachment.srcAccessMask       = VK_ACCESS_NONE; // srcAccessMask   srcAccessMask
    MemoryBarrierSetColorAttachment.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // dstAccessMask   dstAccessMask
    MemoryBarrierSetColorAttachment.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED; // oldLayout   oldLayout
    MemoryBarrierSetColorAttachment.newLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // newLayout   newLayout
    MemoryBarrierSetColorAttachment.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // srcQueueFamilyIndex   srcQueueFamilyIndex
    MemoryBarrierSetColorAttachment.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // dstQueueFamilyIndex   dstQueueFamilyIndex
    MemoryBarrierSetColorAttachment.subresourceRange    = Range; // subresourceRange   subresourceRange

    VkCommandBuffer SingleCmd = BeginSingleCommandBuffer();

    MemoryBarrierSetColorAttachment.image               = GlobalVulkan.WeightedRevealImage->Image; // image   image
    vkCmdPipelineBarrier(SingleCmd, 
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &MemoryBarrierSetColorAttachment);

    MemoryBarrierSetColorAttachment.image               = GlobalVulkan.WeightedColorImage->Image; // image   image
    vkCmdPipelineBarrier(SingleCmd, 
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &MemoryBarrierSetColorAttachment);

    EndSingleCommandBuffer(SingleCmd);


    VULKAN_TREE_APPEND(vkDestroyUnknown, GlobalVulkan.PrimaryDevice, GlobalVulkan.RenderPassLabel);
    if (VulkanInitDefaultRenderPass()) return 1;
    VULKAN_TREE_APPEND(vkDestroyRenderPass, GlobalVulkan.RenderPassLabel, GlobalVulkan.RenderPass);
    if (VulkanCreateTransparentRenderPass()) return 1;
    VULKAN_TREE_APPEND(vkDestroyRenderPass, GlobalVulkan.RenderPassLabel, GlobalVulkan.RenderPassTransparency);

    if (VulkanInitFramebuffers()) return 1;

    // As for now a single sampler for all textures
    VkSamplerCreateInfo SamplerCreateInfo = VulkanSamplerCreateInfo(VK_FILTER_LINEAR,VK_SAMPLER_ADDRESS_MODE_REPEAT);

    VK_CHECK(vkCreateSampler(GlobalVulkan.PrimaryDevice,&SamplerCreateInfo,0,&GlobalVulkan.TextureSampler));
    VULKAN_TREE_APPEND(vkDestroySampler, GlobalVulkan.PrimaryDevice, GlobalVulkan.TextureSampler);

#if 0
    // TODO: how to properly handle dummy texture on initialization?
    // This is to force transition layouts?
    // Why did I do this?
    // I think this was to create a dummy texture during the particle creation testing
    // Plain blue texture of 1x1 size
    i32 DummyImgData = (0xFF << 24) |
                       (0xFF << 0 );

    PushTextureData(&DummyImgData, 1, 1, 4);
#endif

    // Update descriptors once oit weight images are created
    {
        VH_AllocateDescriptor(&GlobalVulkan._oit_WeightedColorAttachmentInputsSetLayout, GlobalVulkan._DescriptorPool, &GlobalVulkan._oit_WeightedColorSet);
        VH_AllocateDescriptor(&GlobalVulkan._oit_WeightedRevealAttachmentInputsSetLayout, GlobalVulkan._DescriptorPool, &GlobalVulkan._oit_WeightedRevealSet);


        VkDescriptorImageInfo OitWeightedColorImageInfo;
        OitWeightedColorImageInfo.sampler     = VK_NULL_HANDLE; // sampler   sampler
        OitWeightedColorImageInfo.imageView   = GlobalVulkan.WeightedColorImage->ImageView; // imageView   imageView
        OitWeightedColorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // imageLayout   imageLayout

        VkWriteDescriptorSet OitWeightedColorWriteSet =
            VH_WriteDescriptor(0,GlobalVulkan._oit_WeightedColorSet, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &OitWeightedColorImageInfo);

        VkDescriptorImageInfo OitWeightedRevealImageInfo;
        OitWeightedRevealImageInfo.sampler     = VK_NULL_HANDLE; // sampler   sampler
        OitWeightedRevealImageInfo.imageView   = GlobalVulkan.WeightedRevealImage->ImageView; // imageView   imageView
        OitWeightedRevealImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // imageLayout   imageLayout

        VkWriteDescriptorSet OitWeightedRevealWriteSet =
            VH_WriteDescriptor(0,GlobalVulkan._oit_WeightedRevealSet, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &OitWeightedRevealImageInfo);

        VkWriteDescriptorSet WriteSets[] = 
        {
            OitWeightedColorWriteSet,
            OitWeightedRevealWriteSet
        };

        vkUpdateDescriptorSets(GlobalVulkan.PrimaryDevice, ArrayCount(WriteSets), WriteSets, 0, nullptr);
    }

    // Other objects not necessarily created during initialization
    VULKAN_TREE_APPEND(vkDestroyUnknown, GlobalVulkan.PrimaryDevice,GlobalVulkan.ShaderModulesLabel); 
    VULKAN_TREE_APPEND(vkDestroyUnknown, GlobalVulkan.PrimaryDevice,GlobalVulkan.PipelinesLabel); 

    GlobalVulkan.Initialized = true;

    return 0;
}

void
VulkanDestroyPipeline()
{
    for (u32 PipelineIndex = 0;
                PipelineIndex < GlobalVulkan.PipelinesCount;
                ++PipelineIndex)
    {
        if (VK_VALID_HANDLE(GlobalVulkan.Pipelines[PipelineIndex]))
        {
            vkDestroyPipeline(GlobalVulkan.PrimaryDevice,GlobalVulkan.Pipelines[PipelineIndex],0);
            GlobalVulkan.Pipelines[PipelineIndex] = VK_NULL_HANDLE;
        }
    }
    GlobalVulkan.PipelinesCount = 0;
}


void
CloseVulkan()
{
    VulkanWaitForDevices();

    HierarchyTreeDropBranch(GlobalVulkan.HTree, GlobalVulkan.Instance);

    free(GlobalVulkan.InitArena.Base);
}

i32 
WaitForRender()
{
    if (GlobalWindowIsMinimized) return 0;

    VK_CHECK(vkWaitForFences(GlobalVulkan.PrimaryDevice, 1, &GetCurrentFrame()->RenderFence, true, 1000000000));
    VK_CHECK(vkResetFences(GlobalVulkan.PrimaryDevice, 1, &GetCurrentFrame()->RenderFence));

    return 0;
}


