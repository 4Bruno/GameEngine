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
 *
 * - Convention value returns
 *   Function names with as actions ('VulkanCreate..')
 *   _always_ return i32 which is:
 *    - Non-zero for error. Where the value is an error-code
 *    - Zero for success
 *   This is _not_ always the case for external API.
 *   Exceptions:
 *    - RenderCreatePipeline
 *    - RenderCreateShaderModule
 *    Will return the internal index in array of the pipeline
 */

#include "vulkan_initializer.h"
// TODO: memcpy. How can we avoid this?
#include <memory.h>

#define VK_FAILS(result) (result != VK_SUCCESS)
#define VK_SUCCESS(result) (result == VK_SUCCESS)
#define VK_VALID_HANDLE(handle) ((handle) != VK_NULL_HANDLE)
#define VK_INVALID_HANDLE(handle) ((handle) == VK_NULL_HANDLE)

#define VK_GLOBAL_LEVEL_FN( fun ) fun = (PFN_##fun)vkGetInstanceProcAddr(0,#fun)
// watch out for the convention name of instance as "Instance"
#define VK_INSTANCE_LEVEL_FN(Instance, fun ) fun = (PFN_##fun)vkGetInstanceProcAddr(Instance,#fun)
#define VK_DEVICE_LEVEL_FN(Device, fun )   fun = (PFN_##fun)vkGetDeviceProcAddr(Device,#fun)

#define RENDER_VERSION      VK_MAKE_VERSION(1,0,0)
#define ENGINE_VERSION      VK_MAKE_VERSION(1,0,0)
/*
 * Why vulkan version?
 * - 1.1.0+ you can pass negative height to surface to use bottom-left coord system
 */
#define VULKAN_API_VERSION  VK_MAKE_VERSION(1,1,0)

#define VK_MEMORY_GPU                       (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
#define VK_MEMORY_CPU_TO_GPU_PREFERRED      (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
#define VK_MEMORY_CPU_TO_GPU_HOST_VISIBLE   (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
#define VK_MEMORY_LARGE_TARGETS_NOT_STORED  (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)

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

PFN_vkCreateSwapchainKHR                      vkCreateSwapchainKHR                      = 0;
PFN_vkDestroySwapchainKHR                     vkDestroySwapchainKHR                     = 0;
PFN_vkGetSwapchainImagesKHR                   vkGetSwapchainImagesKHR                   = 0;
PFN_vkAcquireNextImageKHR                     vkAcquireNextImageKHR                     = 0;
PFN_vkQueuePresentKHR                         vkQueuePresentKHR                         = 0;


PFN_vkEnumeratePhysicalDevices                vkEnumeratePhysicalDevices                = 0;
PFN_vkEnumerateDeviceExtensionProperties      vkEnumerateDeviceExtensionProperties      = 0;
PFN_vkGetPhysicalDeviceProperties             vkGetPhysicalDeviceProperties             = 0;
PFN_vkGetPhysicalDeviceFeatures               vkGetPhysicalDeviceFeatures               = 0;
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

// LOGICAL DEVICE
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

PFN_vkAllocateMemory                          vkAllocateMemory                          = 0;
PFN_vkFreeMemory                              vkFreeMemory                              = 0;
PFN_vkCreateBuffer                            vkCreateBuffer                            = 0;
PFN_vkDestroyBuffer                           vkDestroyBuffer                           = 0;
PFN_vkDestroyImage                            vkDestroyImage                            = 0;
PFN_vkGetBufferMemoryRequirements             vkGetBufferMemoryRequirements             = 0;
PFN_vkGetImageMemoryRequirements              vkGetImageMemoryRequirements              = 0;
PFN_vkMapMemory                               vkMapMemory                               = 0;
PFN_vkUnmapMemory                             vkUnmapMemory                             = 0;
PFN_vkBindBufferMemory                        vkBindBufferMemory                        = 0;
PFN_vkBindImageMemory                         vkBindImageMemory                         = 0;


// vulkan is verbose
// all func calls that return VkResult follow this logic
#define VK_CHECK(FunCall) \
    if (VK_FAILS(FunCall)) \
    { \
        Log("Error during function %s\n",#FunCall); \
        return 1; \
    } \

struct vk_version
{
    u32 Major,Minor,Patch;
};



struct vulkan_pipeline
{
    VkPipelineShaderStageCreateInfo ShaderStages[3];
    u32                          ShaderStagesCount;

    VkPipelineVertexInputStateCreateInfo   VertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo InputAssembly;

    VkPipelineDepthStencilStateCreateInfo DepthStencil;

    VkViewport Viewport;
    VkRect2D   Scissor;

    VkPipelineRasterizationStateCreateInfo Rasterizer;
    VkPipelineColorBlendAttachmentState    ColorBlendAttachment;
    VkPipelineMultisampleStateCreateInfo   Multisampling;

    VkPipelineLayout PipelineLayout;
};


i32
VulkanFindSuitableMemoryIndex(VkPhysicalDevice PhysicalDevice, VkMemoryRequirements MemoryRequirements,VkMemoryPropertyFlags PropertyFlags)
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

struct depth_buffer
{
    VkImage        Image;
    VkImageView    ImageView;
    VkFormat       Format;
    VkDeviceMemory DeviceMemory;
};

struct vulkan_buffer
{
    VkBuffer             Buffer;
    VkDeviceMemory       DeviceMemory;
    VkMemoryRequirements MemoryRequirements;
    VkDevice             DeviceAllocator;
};



VkDescriptorSetLayoutBinding
VulkanCreateDescriptorSetLayoutBinding(u32 BindingSlot,VkDescriptorType DescriptorType,VkShaderStageFlags ShaderStageFlags)
{
    VkDescriptorSetLayoutBinding Result;
    Result.binding            = BindingSlot; // uint32_t   binding;
    Result.descriptorType     = DescriptorType; // VkDescriptorType   descriptorType;
    Result.descriptorCount    = 1; // uint32_t   descriptorCount;
    Result.stageFlags         = ShaderStageFlags; // VkShaderStageFlags   stageFlags;
    Result.pImmutableSamplers = 0; // Typedef * pImmutableSamplers;

    return Result;
}

inline void
DeleteVulkanBuffer(vulkan_buffer * Buffer)
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
VulkanCreateDepthBuffer(VkPhysicalDevice PhysicalDevice,VkDevice Device, VkExtent3D Extent, depth_buffer * DepthBuffer)
{
    VkFormat Format = VK_FORMAT_D32_SFLOAT;
    VkImageUsageFlags Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageCreateInfo ImageCreateInfo = {};

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

    VkImage Image;
    VK_CHECK(vkCreateImage(Device,&ImageCreateInfo, 0, &Image));

    VkMemoryRequirements MemReq;
    vkGetImageMemoryRequirements(Device, Image, &MemReq);
    VkMemoryPropertyFlags PropertyFlags = VK_MEMORY_GPU;

    u32 MemoryTypeIndex = VulkanFindSuitableMemoryIndex(PhysicalDevice,MemReq,PropertyFlags);
    if (MemoryTypeIndex < 0)
    {
        Log("Couldn't find suitable CPU-GPU memory index\n");
        return 1;
    }

    VkDeviceMemory DeviceMemory;
    VkMemoryAllocateInfo AllocateInfo;

    AllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; // VkStructureType   sType;
    AllocateInfo.pNext           = 0;                       // Void * pNext;
    AllocateInfo.allocationSize  = MemReq.size;             // VkDeviceSize allocationSize;
    AllocateInfo.memoryTypeIndex = (u32)MemoryTypeIndex; // u32_t memoryTypeIndex;

    VK_CHECK(vkAllocateMemory(Device, &AllocateInfo, 0, &DeviceMemory));

    vkBindImageMemory(Device,Image,DeviceMemory,0);

    VkImageSubresourceRange SubresourceRange;
    SubresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT; // VkImageAspectFlags   aspectMask;
    SubresourceRange.baseMipLevel   = 0; // u32_t   baseMipLevel;
    SubresourceRange.levelCount     = 1; // u32_t   levelCount;
    SubresourceRange.baseArrayLayer = 0; // u32_t   baseArrayLayer;
    SubresourceRange.layerCount     = 1; // u32_t   layerCount;

    VkImageViewCreateInfo ImageViewCreateInfo = {}; 

    ImageViewCreateInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; // VkStructureType   sType;
    ImageViewCreateInfo.pNext    = 0;                     // Void * pNext;
    ImageViewCreateInfo.flags    = 0;                     // VkImageViewCreateFlags flags;
    ImageViewCreateInfo.image    = Image;                 // VkImage image;
    ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // VkImageViewType viewType;
    ImageViewCreateInfo.format   = Format;                // VkFormat format;
    //ImageViewCreateInfo.components       = ; // VkComponentMapping   components;
    ImageViewCreateInfo.subresourceRange = SubresourceRange; // VkImageSubresourceRange   subresourceRange;

    VkImageView ImageView;
    VK_CHECK(vkCreateImageView( Device, &ImageViewCreateInfo, 0, &ImageView ));

    DepthBuffer->Image        = Image;        // VkImage Buffer;
    DepthBuffer->ImageView    = ImageView;    // VkImageView ImageView;
    DepthBuffer->Format       = Format;       // VkFormat Format;
    DepthBuffer->DeviceMemory = DeviceMemory; // VkFormat Format;

    return 0;
}

i32
VulkanCreateImage(VkPhysicalDevice PhysicalDevice,VkDevice Device, 
                  VkImage * Image,
                  VkDeviceMemory * DeviceMemory, 
                  VkMemoryRequirements * MemReq,
                  u32 Width, u32 Height, u32 Channels)
{
    VkImageCreateInfo ImageCreateInfo;

    ImageCreateInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO; // VkStructureType   sType;
    ImageCreateInfo.pNext                 = 0; // Void * pNext;
    ImageCreateInfo.flags                 = 0; // VkImageCreateFlags   flags;
    ImageCreateInfo.imageType             = VK_IMAGE_TYPE_2D; // VkImageType   imageType;

    Assert((Channels == 3) || (Channels == 4));

#if 0
    if (Channels == 3)
    {
        Assert(0); // CHECK IF SUPPORTED!
        ImageCreateInfo.format                = VK_FORMAT_R8G8B8_UNORM;
    }
    else if (Channels == 4)
    {
        ImageCreateInfo.format                = VK_FORMAT_R8G8B8A8_SRGB; // VkFormat   format;
    }
#endif

    ImageCreateInfo.format                = VK_FORMAT_R8G8B8A8_SRGB; // VkFormat   format;

    ImageCreateInfo.extent.width  = Width;
    ImageCreateInfo.extent.height = Height;
    ImageCreateInfo.extent.depth  = 1;

    ImageCreateInfo.mipLevels             = 1; // uint32_t   mipLevels;
    ImageCreateInfo.arrayLayers           = 1; // uint32_t   arrayLayers;

    ImageCreateInfo.samples               = VK_SAMPLE_COUNT_1_BIT; // VkSampleCountFlagBits   samples;
    ImageCreateInfo.tiling                = VK_IMAGE_TILING_OPTIMAL; // VkImageTiling   tiling;
    ImageCreateInfo.usage                 = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; // VkImageUsageFlags   usage;
    ImageCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE; // VkSharingMode   sharingMode;
    //ImageCreateInfo.queueFamilyIndexCount = ; // uint32_t   queueFamilyIndexCount;
    //ImageCreateInfo.pQueueFamilyIndices   = ; // Typedef * pQueueFamilyIndices;
    ImageCreateInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED; // VkImageLayout   initialLayout;

    VK_CHECK(vkCreateImage(Device,&ImageCreateInfo, 0, Image));

    vkGetImageMemoryRequirements(Device, *Image, MemReq);
    VkMemoryPropertyFlags PropertyFlags = VK_MEMORY_GPU;

    u32 MemoryTypeIndex = VulkanFindSuitableMemoryIndex(PhysicalDevice,*MemReq,PropertyFlags);
    if (MemoryTypeIndex < 0)
    {
        Log("Couldn't find suitable CPU-GPU memory index\n");
        return 1;
    }

    VkMemoryAllocateInfo AllocateInfo;

    AllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; // VkStructureType   sType;
    AllocateInfo.pNext           = 0;                       // Void * pNext;
    AllocateInfo.allocationSize  = MemReq->size;             // VkDeviceSize allocationSize;
    AllocateInfo.memoryTypeIndex = (u32)MemoryTypeIndex; // u32_t memoryTypeIndex;

    VK_CHECK(vkAllocateMemory(Device, &AllocateInfo, 0, DeviceMemory));

    vkBindImageMemory(Device,*Image,*DeviceMemory,0);

    return 0;
}

struct frame_data 
{
    VkFence     RenderFence;
    VkSemaphore ImageAvailableSemaphore;
    VkSemaphore RenderSemaphore;

    VkCommandPool   CommandPool;
    VkCommandBuffer PrimaryCommandBuffer;

    VkDescriptorSet GlobalDescriptor;
    VkDescriptorSet ObjectsDescriptor;

    vulkan_buffer ObjectsBuffer;
    u32 ObjectsCount;
};

// number of frames  [0-2] we are looping
#define FRAME_OVERLAP 2

struct vulkan
{
    b32 Initialized;

    VkInstance       Instance;

    // Primary GPU
    VkPhysicalDevice PrimaryGPU;
    VkDevice         PrimaryDevice;

    depth_buffer DepthBuffer;

    u32  GraphicsQueueFamilyIndex;
    VkQueue GraphicsQueue;
    u32  PresentationQueueFamilyIndex;
    VkQueue PresentationQueue;
    u32  TransferOnlyQueueFamilyIndex;
    VkQueue TransferOnlyQueue;

    VkCommandPool   CommandPoolTransferBit;
    VkCommandBuffer TransferBitCommandBuffer;

    vulkan_buffer TransferBitBuffer;

    VkDeviceMemory TextureDeviceMemory;
    VkImage        TextureImage;
    VkImageView    TextureImageView;
    VkSampler      TextureSampler;
    VkDeviceSize   TextureMemAlign;

    vulkan_buffer VertexBuffer;
    vulkan_buffer IndexBuffer;

    VkRenderPass  RenderPass;
    VkFramebuffer Framebuffers[3];

    VkDescriptorSetLayout _GlobalSetLayout;
    VkDescriptorSetLayout _ObjectsSetLayout;
    VkDescriptorPool _DescriptorPool;

    // Global buffer for simulation data to
    // be passed to shaders
    vulkan_buffer SimulationBuffer;

    frame_data FrameData[FRAME_OVERLAP];
    i32 _CurrentFrameData;

    // Secondary GPU if available
    VkPhysicalDevice SecondaryGPU;
    VkDevice         SecondaryDevice;

    // Presentation
    VkSurfaceKHR   Surface;
    VkSwapchainKHR Swapchain;
    VkFormat       SwapchainImageFormat;
    VkExtent2D     WindowExtension;
    VkImage        SwapchainImages[3];
    VkImageView    SwapchainImageViews[3];
    u32         SwapchainImageCount;
    u32         CurrentSwapchainImageIndex;

    // NOTE: is this app specific?
    VkPipelineLayout PipelineLayout;


    VkPipeline      Pipelines[2];
    vulkan_pipeline PipelinesDefinition[2];
    u32          PipelinesCount;

    VkShaderModule ShaderModules[4];
    u32         ShaderModulesCount;
};

global_variable vulkan        GlobalVulkan    = {};
global_variable b32        GlobalWindowIsMinimized = false;


VkDeviceSize
PaddedUniformBuffer(VkDeviceSize Size)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(GlobalVulkan.PrimaryGPU, &properties);
    VkDeviceSize Align = 
        (properties.limits.minUniformBufferOffsetAlignment - 1);

    VkDeviceSize AlignedSize = (Size + Align) & ~Align;

    return AlignedSize;
}

VkPipelineShaderStageCreateInfo
VulkanCreateShaderStageInfo(VkShaderStageFlagBits Stage, VkShaderModule Module)
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

VkPipelineVertexInputStateCreateInfo
VulkanCreateVertexInputStateInfo()
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
VulkanCreatePipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology Topology)
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
VulkanCreatePipelineRasterizationStateCreateInfo(VkPolygonMode PolygonMode)
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
VulkanCreatePipelineMultisampleStateCreateInfo()
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
VulkanCreatePipelineColorBlendAttachmentState()
{
    VkPipelineColorBlendAttachmentState PipelineColorBlendAttachmentState = {};

#if 1
    PipelineColorBlendAttachmentState.blendEnable         = VK_FALSE; // VkBool32   blendEnable;
#else
    PipelineColorBlendAttachmentState.blendEnable         = VK_TRUE; // VkBool32   blendEnable;
    PipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;           // VkBlendFactor srcColorBlendFactor;
    PipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // VkBlendFactor dstColorBlendFactor;
    PipelineColorBlendAttachmentState.colorBlendOp        = VK_BLEND_OP_ADD;                     // VkBlendOp colorBlendOp;
    PipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;                 // VkBlendFactor srcAlphaBlendFactor;
    PipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;                // VkBlendFactor dstAlphaBlendFactor;
    PipelineColorBlendAttachmentState.alphaBlendOp        = VK_BLEND_OP_ADD;                      // VkBlendOp alphaBlendOp;
#endif
    PipelineColorBlendAttachmentState.colorWriteMask      = 
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // VkColorComponentFlags   colorWriteMask;

    return PipelineColorBlendAttachmentState;
};


VkPipelineLayoutCreateInfo
VulkanCreatePipelineLayoutCreateInfo()
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

VkPipeline
VulkanPipelineBuilder(vulkan_pipeline * VulkanPipeline,VkDevice Device, VkRenderPass RenderPass)
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
    ColorBlending.attachmentCount = 1;                                     // u32_t attachmentCount;
    ColorBlending.pAttachments    = &VulkanPipeline->ColorBlendAttachment; // Typedef * pAttachments;
    //ColorBlending.blendConstants  = 0;// CONSTANTARRAY blendConstants;

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
    PipelineInfo.pDynamicState       = 0;                                 // Typedef * pDynamicState;
    PipelineInfo.layout              = VulkanPipeline->PipelineLayout;    // VkPipelineLayout layout;
    PipelineInfo.renderPass          = RenderPass;                        // VkRenderPass renderPass;
    PipelineInfo.subpass             = 0;                                 // u32_t subpass;
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

VkViewport
VulkanCreateDefaultViewport(VkExtent2D WindowExtent)
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


i32
VulkanReCreatePipelinesOnWindowResize()
{
    i32 Result = 0;

    for (u32 PipelineIndex = 0;
            PipelineIndex < GlobalVulkan.PipelinesCount;
            ++PipelineIndex)
    {
        if (VK_VALID_HANDLE(GlobalVulkan.Pipelines[PipelineIndex]))
        {
            vkDestroyPipeline(GlobalVulkan.PrimaryDevice,GlobalVulkan.Pipelines[PipelineIndex],0);

            GlobalVulkan.Pipelines[PipelineIndex] = VK_NULL_HANDLE;

            vulkan_pipeline * VulkanPipeline = (GlobalVulkan.PipelinesDefinition + PipelineIndex);

            vertex_inputs_description VertexInputDesc = RenderGetVertexInputsDescription();
            VulkanPipeline->VertexInputInfo.vertexBindingDescriptionCount   = 1;                              // u32_t vertexBindingDescriptionCount;
            VulkanPipeline->VertexInputInfo.pVertexBindingDescriptions      = &VertexInputDesc.Bindings[0];   // Typedef * pVertexBindingDescriptions;
            VulkanPipeline->VertexInputInfo.vertexAttributeDescriptionCount = ArrayCount(VertexInputDesc.Attributes);                              // u32_t vertexAttributeDescriptionCount;
            VulkanPipeline->VertexInputInfo.pVertexAttributeDescriptions    = &VertexInputDesc.Attributes[0]; // Typedef * pVertexAttributeDescriptions;

            VulkanPipeline->Viewport       = VulkanCreateDefaultViewport(GlobalVulkan.WindowExtension); // VkViewport Viewport;

            VulkanPipeline->Scissor.offset = {0,0};
            VulkanPipeline->Scissor.extent = GlobalVulkan.WindowExtension;

            VkPipeline Pipeline = VulkanPipelineBuilder(VulkanPipeline, GlobalVulkan.PrimaryDevice, GlobalVulkan.RenderPass);

            if (VK_VALID_HANDLE(Pipeline))
            {
                GlobalVulkan.Pipelines[PipelineIndex] = Pipeline;
            }
            else
            {
                Log("Failed to re-create pipeline '%i' after window resize\n",PipelineIndex);
                Result = 1;
            }
        }
    }

    return Result;
}


pipeline_creation_result
RenderCreatePipeline(i32 VertexShaderIndex,
                     i32 FragmentShaderIndex)
{

    pipeline_creation_result Result = {};

    Assert((VertexShaderIndex >= 0) && (ArrayCount(GlobalVulkan.ShaderModules) > VertexShaderIndex));
    Assert((FragmentShaderIndex >= 0) && (ArrayCount(GlobalVulkan.ShaderModules) > FragmentShaderIndex));

    VkShaderModule VertexShader = GlobalVulkan.ShaderModules[VertexShaderIndex];
    VkShaderModule FragmentShader = GlobalVulkan.ShaderModules[FragmentShaderIndex];

    vulkan_pipeline VulkanPipeline;

    VulkanPipeline.ShaderStagesCount = 2; // u32 ShaderStagesCount;
    VulkanPipeline.ShaderStages[0]   = VulkanCreateShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT,VertexShader);
    VulkanPipeline.ShaderStages[1]   = VulkanCreateShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT,FragmentShader);

    VulkanPipeline.VertexInputInfo      = VulkanCreateVertexInputStateInfo(); // VkPipelineVertexInputStateCreateInfo   VertexInputInfo;

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
        VulkanCreatePipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST); // VkPipelineInputAssemblyStateCreateInfo   InputAssembly;
#else
    //VulkanPipeline.InputAssembly        = VulkanCreatePipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_LINE_LIST); // VkPipelineInputAssemblyStateCreateInfo   InputAssembly;
    VulkanPipeline.InputAssembly        = VulkanCreatePipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY); // VkPipelineInputAssemblyStateCreateInfo   InputAssembly;
#endif

    VulkanPipeline.Viewport             = VulkanCreateDefaultViewport(GlobalVulkan.WindowExtension); // VkViewport   Viewport;

    VulkanPipeline.Scissor.offset = {0,0};
    VulkanPipeline.Scissor.extent = GlobalVulkan.WindowExtension;

    VulkanPipeline.Rasterizer           = 
        VulkanCreatePipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL); // VkPipelineRasterizationStateCreateInfo   Rasterizer;

    VulkanPipeline.ColorBlendAttachment = VulkanCreatePipelineColorBlendAttachmentState();  // VkPipelineColorBlendAttachmentState ColorBlendAttachment;
    VulkanPipeline.Multisampling        = VulkanCreatePipelineMultisampleStateCreateInfo(); // VkPipelineMultisampleStateCreateInfo Multisampling;
    VulkanPipeline.PipelineLayout       = GlobalVulkan.PipelineLayout;                      // VkPipelineLayout PipelineLayout;

    VkPipeline Pipeline = VulkanPipelineBuilder(&VulkanPipeline, GlobalVulkan.PrimaryDevice, GlobalVulkan.RenderPass);

    if (VK_VALID_HANDLE(Pipeline))
    {
        Assert(GlobalVulkan.PipelinesCount < ArrayCount(GlobalVulkan.Pipelines));

        i32 IndexPipeline = GlobalVulkan.PipelinesCount++;

        GlobalVulkan.PipelinesDefinition[IndexPipeline] = VulkanPipeline;
        GlobalVulkan.Pipelines[IndexPipeline]= Pipeline;

        Result.Pipeline = IndexPipeline;
        Result.Success = true;
        Result.PipelineLayout = 0; // only 1 for now, hardcoded as the first 
    }

    return Result;
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






struct vulkan_device_extensions
{
    const char * DeviceExtensions[1];
    u32 Count;
};

vulkan_device_extensions
GetRequiredDeviceExtensions()
{
    vulkan_device_extensions Ext;
    Ext.DeviceExtensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME; // CONSTANTARRAY   DeviceExtensions;
    Ext.Count               = 1;                               // u32   Count;
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

inline frame_data *
GetCurrentFrame()
{
    frame_data * FrameData = GlobalVulkan.FrameData + (GlobalVulkan._CurrentFrameData % FRAME_OVERLAP);
    
    return FrameData;
}

GPUObjectData *
VulkanBeginObjectDataMapping(u32 * CurrentObjectCount)
{
    void * WriteToAddr = 0;

    VkMemoryMapFlags Flags = 0; // RESERVED FUTURE USE

    *CurrentObjectCount = GetCurrentFrame()->ObjectsCount;

    VkDeviceSize OffsetObjects = 
        GetCurrentFrame()->ObjectsCount * sizeof(GPUObjectData);

    VkDeviceSize Size =
        GetCurrentFrame()->ObjectsBuffer.MemoryRequirements.size - OffsetObjects;

    if (VK_FAILS(vkMapMemory(GlobalVulkan.PrimaryDevice,
                         GetCurrentFrame()->ObjectsBuffer.DeviceMemory, 
                         OffsetObjects, Size, Flags , &WriteToAddr))
       )
    {
        Logn("Failed to initiate mapping on Objects buffer");
        *CurrentObjectCount = 0;
    }

    return (GPUObjectData *)WriteToAddr;
}

void
VulkanEndObjectDataMapping(u32 CountObjects)
{
    vkUnmapMemory(GlobalVulkan.PrimaryDevice,GetCurrentFrame()->ObjectsBuffer.DeviceMemory);

    GetCurrentFrame()->ObjectsCount = CountObjects;
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

    if ( SurfaceCapabilities.currentExtent.width == -1 )
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
        vkDestroySwapchainKHR(GlobalVulkan.PrimaryDevice, OldSwapChain, 0);
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

    }

    return 0;
}

i32
VulkanInitDefaultRenderPass()
{
    VkAttachmentDescription DepthAttachment;
    DepthAttachment.flags          = 0;                                                // VkDepthAttachmentFlags flags;
    DepthAttachment.format         = GlobalVulkan.DepthBuffer.Format;                  // VkFormat format;
    DepthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;                            // VkSampleCountFlagBits samples;
    DepthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;                      // VkAttachmentLoadOp loadOp;
    DepthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;                     // VkAttachmentStoreOp storeOp;
    DepthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;                      // VkAttachmentLoadOp stencilLoadOp;
    DepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                 // VkAttachmentStoreOp stencilStoreOp;
    DepthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;                        // VkImageLayout initialLayout;
    DepthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // VkImageLayout finalLayout;

    VkAttachmentReference DepthAttachmentReference;
    DepthAttachmentReference.attachment = 1; // u32_t   attachment;
    DepthAttachmentReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // VkImageLayout   layout;


    VkAttachmentDescription ColorAttachment;
    ColorAttachment.flags          = 0;                                 // VkColorAttachmentFlags   flags;
    ColorAttachment.format         = GlobalVulkan.SwapchainImageFormat; // VkFormat   format;
    ColorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;             // VkSampleCountFlagBits   samples;
    ColorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;       // VkAttachmentLoadOp   loadOp;
    ColorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;      // VkAttachmentStoreOp   storeOp;
    ColorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   // VkAttachmentLoadOp   stencilLoadOp;
    ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // VkAttachmentStoreOp   stencilStoreOp;
    ColorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;         // VkImageLayout   initialLayout;
    ColorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;   // VkImageLayout   finalLayout;

    VkAttachmentReference ColorAttachmentReference;
    ColorAttachmentReference.attachment = 0;                                        // u32_t   attachment;
    ColorAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VkImageLayout   layout;

    VkSubpassDescription SubpassDescription;
    SubpassDescription.flags                   = 0;                               // VkSubpassDescriptionFlags flags;
    SubpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS; // VkPipelineBindPoint pipelineBindPoint;
    SubpassDescription.inputAttachmentCount    = 0;                               // u32_t inputAttachmentCount;
    SubpassDescription.pInputAttachments       = 0;                               // Typedef * pInputAttachments;
    SubpassDescription.colorAttachmentCount    = 1;                               // u32_t colorAttachmentCount;
    SubpassDescription.pColorAttachments       = &ColorAttachmentReference;       // Typedef * pColorAttachments;
    SubpassDescription.pResolveAttachments     = 0;                               // Typedef * pResolveAttachments;
    SubpassDescription.pDepthStencilAttachment = &DepthAttachmentReference;       // Typedef * pDepthStencilAttachment;
    SubpassDescription.preserveAttachmentCount = 0;                               // u32_t preserveAttachmentCount;
    SubpassDescription.pPreserveAttachments    = 0;                               // Typedef * pPreserveAttachments;

    VkAttachmentDescription Attachments[2] = {
        ColorAttachment,
        DepthAttachment
    };

#if 0
    VkSubpassDependency Dependency;
    //Dependency.srcSubpass      = ; // uint32_t   srcSubpass;
    //Dependency.dstSubpass      = ; // uint32_t   dstSubpass;
    Dependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // VkPipelineStageFlags   srcStageMask;
    Dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;; // VkPipelineStageFlags   dstStageMask;
    //Dependency.srcAccessMask   = ; // VkAccessFlags   srcAccessMask;
    Dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;; // VkAccessFlags   dstAccessMask;
    //Dependency.dependencyFlags = ; // VkDependencyFlags   dependencyFlags;
#endif

    VkRenderPassCreateInfo RenderPassCreateInfo;
    RenderPassCreateInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO; // VkStructureType sType;
    RenderPassCreateInfo.pNext           = 0;                                         // Void * pNext;
    RenderPassCreateInfo.flags           = 0;                                         // VkRenderPassCreateFlags flags;
    RenderPassCreateInfo.attachmentCount = ArrayCount(Attachments);                   // u32_t attachmentCount;
    RenderPassCreateInfo.pAttachments    = &Attachments[0];                           // Typedef * pAttachments;
    RenderPassCreateInfo.subpassCount    = 1;                                         // u32_t subpassCount;
    RenderPassCreateInfo.pSubpasses      = &SubpassDescription;                       // Typedef * pSubpasses;
#if 0
    RenderPassCreateInfo.dependencyCount = 1;                                         // u32_t dependencyCount;
    RenderPassCreateInfo.pDependencies   = &Dependency;                               // Typedef * pDependencies;
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

    for (u32 ImageIndex = 0;
                ImageIndex < GlobalVulkan.SwapchainImageCount;
                ++ImageIndex)
    {
        VkImageView Attachments[2] = {
            GlobalVulkan.SwapchainImageViews[ImageIndex],
            GlobalVulkan.DepthBuffer.ImageView
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

    return 0;
}

i32
VulkanCreateCommandPool(VkDevice Device,
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
VulkanCreateCommandBuffers(VkDevice Device,VkCommandPool CommandPool,u32 CommandBufferCount,VkCommandBuffer * CommandBuffers)
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

i32
WaitForRender()
{
    if (GlobalWindowIsMinimized) return 0;

    VK_CHECK(vkWaitForFences(GlobalVulkan.PrimaryDevice, 1, &GetCurrentFrame()->RenderFence, true, 1000000000));
    VK_CHECK(vkResetFences(GlobalVulkan.PrimaryDevice, 1, &GetCurrentFrame()->RenderFence));

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

    if (VK_SUCCESS(Result))
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


i32
BeginCommandBuffer(VkCommandBuffer CommandBuffer)
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
EndCommandBuffer(VkCommandBuffer CommandBuffer, VkQueue FamilyQueue)
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
VulkanCopyBuffer(VkCommandBuffer CommandBuffer, VkBuffer Src, VkBuffer Dest, VkDeviceSize Size, VkDeviceSize Offset)
{
    BeginCommandBuffer(CommandBuffer);

    VkBufferCopy CopyRegion = {};
    CopyRegion.srcOffset = 0;    // VkDeviceSize srcOffset;
    CopyRegion.dstOffset = Offset;    // VkDeviceSize dstOffset;
    CopyRegion.size      = Size; // VkDeviceSize size;

    vkCmdCopyBuffer(CommandBuffer, Src, Dest, 1, &CopyRegion);

    EndCommandBuffer(CommandBuffer,GlobalVulkan.TransferOnlyQueue);

    return 0;
}


i32
VulkanCreateBuffer(VkPhysicalDevice PhysicalDevice,VkDevice Device, 
                    VkDeviceSize Size, 
                    VkSharingMode SharingMode,
                    VkMemoryPropertyFlags PropertyFlags, 
                    VkBufferUsageFlags Usage,
                    vulkan_buffer * Buffer,
                    u32 SharedBufferQueueFamilyIndexCount = 0,
                    u32 * SharedBufferQueueFamilyIndexArray = 0)
{
    Buffer->DeviceAllocator = Device; 

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

    VK_CHECK(vkCreateBuffer(Device, &BufferCreateInfo,0, &Buffer->Buffer));

    vkGetBufferMemoryRequirements(Device, Buffer->Buffer, &Buffer->MemoryRequirements);

    u32 MemoryTypeIndex = VulkanFindSuitableMemoryIndex(PhysicalDevice,Buffer->MemoryRequirements,PropertyFlags);
    if (MemoryTypeIndex < 0)
    {
#if 0
        PropertyFlags = VK_MEMORY_CPU_TO_GPU_NOTBAD;
        MemoryTypeIndex = VulkanFindSuitableMemoryIndex(PhysicalDevice,MemReq,PropertyFlags);
        if (MemoryTypeIndex < 0)
        {
            Log("Couldn't find suitable CPU-GPU memory index\n");
            return 1;
        }
#endif
        Log("Couldn't find suitable CPU-GPU memory index\n");
        return 1;
    }

    VkMemoryAllocateInfo AllocateInfo;

    AllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; // VkStructureType   sType;
    AllocateInfo.pNext           = 0;                       // Void * pNext;
    AllocateInfo.allocationSize  = Buffer->MemoryRequirements.size;             // VkDeviceSize allocationSize;
    AllocateInfo.memoryTypeIndex = (u32)MemoryTypeIndex; // u32_t memoryTypeIndex;

    VK_CHECK(vkAllocateMemory(Device, &AllocateInfo, 0, &Buffer->DeviceMemory));

    vkBindBufferMemory(Device,Buffer->Buffer,Buffer->DeviceMemory,0);

    return 0;
}


memory_arena
RenderGetMemoryArena()
{
    memory_arena Arena = {};
    Arena.Base             = 0; // Typedef * Base;
    Arena.MaxSize          = Megabytes(100); // u32   MaxSize;
    Arena.CurrentSize      = 0; // u32   CurrentSize;

    return Arena;
}

void
RenderPushVertexConstant(u32 Size,void * Data)
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;

    vkCmdPushConstants(cmd,GlobalVulkan.PipelineLayout,VK_SHADER_STAGE_VERTEX_BIT,0,Size,Data);
}

u32
RenderGetVertexMemAlign()
{
    u32 Align = (u32)GlobalVulkan.VertexBuffer.MemoryRequirements.alignment;
    return  Align;
}

void
DestroyTextures()
{
    if (VK_VALID_HANDLE(GlobalVulkan.TextureSampler))
    {
        vkDestroySampler(GlobalVulkan.PrimaryDevice,GlobalVulkan.TextureSampler,0);
    }
    if (VK_VALID_HANDLE(GlobalVulkan.TextureImageView))
    {
        vkDestroyImageView(GlobalVulkan.PrimaryDevice,GlobalVulkan.TextureImageView,0);
    }

    if (VK_VALID_HANDLE(GlobalVulkan.TextureImage))
    {
        vkDestroyImage(GlobalVulkan.PrimaryDevice,GlobalVulkan.TextureImage,0);
    }

    if (VK_VALID_HANDLE(GlobalVulkan.TextureDeviceMemory))
    {
        vkFreeMemory(GlobalVulkan.PrimaryDevice, GlobalVulkan.TextureDeviceMemory, 0);
    }
}

i32
VulkanPushTexture(void * Data, u32 Width, u32 Height, u32 BaseOffset, u32 Channels)
{
    VkDeviceSize Offset = BaseOffset;

    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkMemoryMapFlags.html
    VkMemoryMapFlags Flags = 0; // RESERVED FUTURE USE

    void * WriteToAddr;

    DestroyTextures();

    VkMemoryRequirements MemReq;

    if (VulkanCreateImage(
                GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice, 
                &GlobalVulkan.TextureImage , &GlobalVulkan.TextureDeviceMemory, &MemReq,
                Width,Height,Channels)) 
        return 1;

    GlobalVulkan.TextureMemAlign = MemReq.alignment;

    VkDeviceSize DeviceSize = MemReq.size;
    DeviceSize = Width * Height * Channels;

    /*
     * 1) Copy CPU to GPU visible memory
     * 2) Transition Image as destination
     * 3) Copy GPU temp memory to Image
     * 4) Transition Image to readable by shaders
     */
    VK_CHECK(vkMapMemory(GlobalVulkan.PrimaryDevice,
                         GlobalVulkan.TransferBitBuffer.DeviceMemory, 
                         0 /*Offset*/, DeviceSize, Flags , &WriteToAddr));

    memcpy(WriteToAddr, Data, DeviceSize);

    vkUnmapMemory(GlobalVulkan.PrimaryDevice,GlobalVulkan.TransferBitBuffer.DeviceMemory);


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
    TransferBarrier.image               = GlobalVulkan.TextureImage;
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

    BeginCommandBuffer(GlobalVulkan.TransferBitCommandBuffer);


	vkCmdCopyBufferToImage(GlobalVulkan.TransferBitCommandBuffer, 
                           GlobalVulkan.TransferBitBuffer.Buffer, 
                           GlobalVulkan.TextureImage, 
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Copy);

    EndCommandBuffer(GlobalVulkan.TransferBitCommandBuffer, GlobalVulkan.TransferOnlyQueue);

    /* TRANSITION LAYOUT */
    VkImageMemoryBarrier ReadableBarrier = {};
    ReadableBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ReadableBarrier.oldLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    ReadableBarrier.newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ReadableBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ReadableBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ReadableBarrier.image               = GlobalVulkan.TextureImage;
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
    ImageViewCreateInfo.image            = GlobalVulkan.TextureImage; // VkImage   image;
    ImageViewCreateInfo.viewType         = VK_IMAGE_VIEW_TYPE_2D; // VkImageViewType   viewType;
    ImageViewCreateInfo.format           = VK_FORMAT_R8G8B8A8_SRGB; // VkFormat   format;

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

    VK_CHECK(vkCreateImageView( GlobalVulkan.PrimaryDevice, &ImageViewCreateInfo, 0, &GlobalVulkan.TextureImageView));

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(GlobalVulkan.PrimaryGPU, &properties);
    r32 MaxSamplerAnisotropy = properties.limits.maxSamplerAnisotropy;

    VkSamplerCreateInfo SamplerCreateInfo;

    SamplerCreateInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO; // VkStructureType   sType;
    SamplerCreateInfo.pNext                   = 0; // Void * pNext;
    SamplerCreateInfo.flags                   = 0; // VkSamplerCreateFlags   flags;
    SamplerCreateInfo.magFilter               = VK_FILTER_LINEAR; // VkFilter   magFilter;
    SamplerCreateInfo.minFilter               = VK_FILTER_LINEAR; // VkFilter   minFilter;
    SamplerCreateInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR; // VkSamplerMipmapMode   mipmapMode;
    SamplerCreateInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT; // VkSamplerAddressMode   addressModeU;
    SamplerCreateInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT; // VkSamplerAddressMode   addressModeV;
    SamplerCreateInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT; // VkSamplerAddressMode   addressModeW;
    SamplerCreateInfo.mipLodBias              = 0.0f; // FLOAT   mipLodBias;
    SamplerCreateInfo.anisotropyEnable        = VK_TRUE; // VkBool32   anisotropyEnable;
    SamplerCreateInfo.maxAnisotropy           = MaxSamplerAnisotropy; // FLOAT   maxAnisotropy;
    SamplerCreateInfo.compareEnable           = VK_FALSE; // VkBool32   compareEnable;
    SamplerCreateInfo.compareOp               = VK_COMPARE_OP_ALWAYS; // VkCompareOp   compareOp;
    SamplerCreateInfo.minLod                  = 0.0f; // FLOAT   minLod;
    SamplerCreateInfo.maxLod                  = 0.0f; // FLOAT   maxLod;
    SamplerCreateInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // VkBorderColor   borderColor;
    SamplerCreateInfo.unnormalizedCoordinates = VK_FALSE; // VkBool32   unnormalizedCoordinates;

    VK_CHECK(vkCreateSampler(GlobalVulkan.PrimaryDevice,&SamplerCreateInfo,0,&GlobalVulkan.TextureSampler));

    return 0;
}

i32
RenderPushVertexData(void * Data, u32 DataSize, u32 BaseOffset)
{
    VkDeviceSize Offset = BaseOffset;
    VkDeviceSize DeviceSize = DataSize;

    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkMemoryMapFlags.html
    VkMemoryMapFlags Flags = 0; // RESERVED FUTURE USE

    void * WriteToAddr;

    VK_CHECK(vkMapMemory(GlobalVulkan.PrimaryDevice,
                         GlobalVulkan.TransferBitBuffer.DeviceMemory, 0 /*Offset*/, DeviceSize, Flags , &WriteToAddr));

    memcpy(WriteToAddr, Data, DataSize);

    vkUnmapMemory(GlobalVulkan.PrimaryDevice,GlobalVulkan.TransferBitBuffer.DeviceMemory);

    if (VulkanCopyBuffer(GlobalVulkan.TransferBitCommandBuffer, 
                     GlobalVulkan.TransferBitBuffer.Buffer,GlobalVulkan.VertexBuffer.Buffer, DataSize, BaseOffset))
    {
        Log("Failed to copy data from buffer to gpu\n");
        return 1;
    }

    return 0;
}


i32
RenderPushIndexData(void * Data,u32 DataSize, u32 BaseOffset)
{
    VkDeviceSize Offset = BaseOffset;
    VkDeviceSize DeviceSize = DataSize;

    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkMemoryMapFlags.html
    VkMemoryMapFlags Flags = 0; // RESERVED FUTURE USE

    void * WriteToAddr;

    VK_CHECK(vkMapMemory(GlobalVulkan.PrimaryDevice,
                         GlobalVulkan.TransferBitBuffer.DeviceMemory, 0 /* offset */, DeviceSize, Flags , &WriteToAddr));

    memcpy(WriteToAddr, Data, DataSize);

    vkUnmapMemory(GlobalVulkan.PrimaryDevice,GlobalVulkan.TransferBitBuffer.DeviceMemory);


    if (VulkanCopyBuffer(GlobalVulkan.TransferBitCommandBuffer, 
                     GlobalVulkan.TransferBitBuffer.Buffer,GlobalVulkan.IndexBuffer.Buffer, DataSize, BaseOffset))
    {
        Log("Failed to copy data from buffer to gpu\n");
        return 1;
    }

    return 0;
}

i32
RenderFreeShaders()
{
    for (u32 ShaderIndex = 0;
                ShaderIndex < GlobalVulkan.ShaderModulesCount;
                ++ShaderIndex)
    {
        Assert(VK_VALID_HANDLE(GlobalVulkan.ShaderModules[ShaderIndex]));
        vkDestroyShaderModule(GlobalVulkan.PrimaryDevice,GlobalVulkan.ShaderModules[ShaderIndex],0);
        GlobalVulkan.ShaderModules[ShaderIndex] = VK_NULL_HANDLE;
    }
    GlobalVulkan.ShaderModulesCount = 0;

    return 0;
}

i32
RenderCreateShaderModule(char * Buffer, size_t Size)
{

    i32 ShaderIndex = -1;
    
    VkShaderModule ShaderModule;

    Assert((GlobalVulkan.ShaderModulesCount + 1) <= ArrayCount(GlobalVulkan.ShaderModules));

    VkShaderModuleCreateInfo ShaderModuleCreateInfo;

    ShaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO; // VkStructureType sType;
    ShaderModuleCreateInfo.pNext    = 0;                                           // Void * pNext;
    ShaderModuleCreateInfo.flags    = 0;                                           // VkShaderModuleCreateFlags flags;
    ShaderModuleCreateInfo.codeSize = Size;                                        // size_t codeSize;
    ShaderModuleCreateInfo.pCode    = (u32 *)Buffer;                            // Typedef * pCode;

    if (VK_SUCCESS(vkCreateShaderModule(GlobalVulkan.PrimaryDevice, &ShaderModuleCreateInfo, 0, &ShaderModule)))
    {
        ShaderIndex = GlobalVulkan.ShaderModulesCount++;
        GlobalVulkan.ShaderModules[ShaderIndex] = ShaderModule;
    }

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
DeleteFrameDataObjectsBuffer()
{
    for (i32 FrameIndex = 0;
            FrameIndex < FRAME_OVERLAP;
            ++FrameIndex)
    {
        frame_data * FrameData = GlobalVulkan.FrameData + FrameIndex;
        DeleteVulkanBuffer(&FrameData->ObjectsBuffer);
    }
}

i32
AllocateFrameDataObjectsBuffer(u32 TotalObjects)
{
    u32 ObjectsSize = sizeof(GPUObjectData) * TotalObjects;

    for (i32 FrameIndex = 0;
            FrameIndex < FRAME_OVERLAP;
            ++FrameIndex)
    {
        frame_data * FrameData = GlobalVulkan.FrameData + FrameIndex;
        if (VulkanCreateBuffer(
                    GlobalVulkan.PrimaryGPU,
                    GlobalVulkan.PrimaryDevice, 
                    ObjectsSize, 
                    VK_SHARING_MODE_EXCLUSIVE,
                    VK_MEMORY_CPU_TO_GPU_PREFERRED,
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    &FrameData->ObjectsBuffer))
        {
            return 1;
        }
    }

    return 0;
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

    return 0;
}

i32
RenderSetPipeline(i32 PipelineIndex)
{
    Assert((PipelineIndex >= 0) && ((u32)PipelineIndex < GlobalVulkan.PipelinesCount));
    Assert(VK_VALID_HANDLE(GlobalVulkan.Pipelines[PipelineIndex]));

    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;

    VkPipeline Pipeline = GlobalVulkan.Pipelines[PipelineIndex];

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);

    u32 CurrentFrameIndex = GlobalVulkan._CurrentFrameData % FRAME_OVERLAP;
    u32 UniformOffset = (u32)PaddedUniformBuffer(sizeof(GPUSimulationData)) * CurrentFrameIndex;

    VkPipelineLayout PipelineLayout = 
        GlobalVulkan.PipelinesDefinition[PipelineIndex].PipelineLayout;

    /*
    uint32_t firstSet, 
    uint32_t descriptorSetCount, 
    VkDescriptorSet*, 
    uint32_t dynamicOffsetCount, 
    const uint32_t* pDynamicOffsets
    */
    u32 FirstSet = 0;
    vkCmdBindDescriptorSets(cmd, 
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            PipelineLayout, 
                            FirstSet, 1, 
                            &GetCurrentFrame()->GlobalDescriptor, 
                            1, 
                            &UniformOffset);


    FirstSet = 1;
    vkCmdBindDescriptorSets(cmd, 
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            PipelineLayout, 
                            FirstSet, 1, 
                            &GetCurrentFrame()->ObjectsDescriptor, 
                            0, nullptr);

    return 0;
}

i32
RenderPushMeshIndexed(u32 TotalMeshInstances, u32 IndicesSize, VkDeviceSize OffsetVertex, VkDeviceSize OffsetIndices)
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;

    vkCmdBindVertexBuffers(cmd, 0, 1, &GlobalVulkan.VertexBuffer.Buffer, &OffsetVertex);
    vkCmdBindIndexBuffer(cmd, GlobalVulkan.IndexBuffer.Buffer, OffsetIndices, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd,IndicesSize,1,0,0,0);

    return 0;
}
i32
RenderDrawObject(u32 VertexSize,u32 FirstInstance)
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;

    vkCmdDraw(cmd,VertexSize, 1, 0 , FirstInstance);

    return 0;
}

i32
RenderBindMesh(u32 VertexSize, u32 Offset)
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;

    VkDeviceSize OffsetVertex = Offset;
    vkCmdBindVertexBuffers(cmd, 0, 1, &GlobalVulkan.VertexBuffer.Buffer, &OffsetVertex);

    return 0;
}
i32
RenderPushSimulationData(GPUSimulationData * SimData)
{
    void * WriteToAddr;

    u32 CurrentFrame = GlobalVulkan._CurrentFrameData % FRAME_OVERLAP;

    VkDeviceSize DeviceSize = sizeof(GPUSimulationData);

    VkDeviceSize Offset = 
        PaddedUniformBuffer(sizeof(GPUSimulationData)) * CurrentFrame;

    VkMemoryMapFlags Flags = 0; // RESERVED FUTURE USE

    VK_CHECK(vkMapMemory(GlobalVulkan.PrimaryDevice,
                         GlobalVulkan.SimulationBuffer.DeviceMemory, 
                         Offset , DeviceSize, Flags , &WriteToAddr));

    memcpy(WriteToAddr, (void *)SimData, DeviceSize);

    vkUnmapMemory(GlobalVulkan.PrimaryDevice,GlobalVulkan.SimulationBuffer.DeviceMemory);

    return 0;
}


i32
RenderEndPass()
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;
    u32 SwapchainImageIndex = GlobalVulkan.CurrentSwapchainImageIndex;

    vkCmdEndRenderPass(cmd);

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

    ++GlobalVulkan._CurrentFrameData;

    return 0;
}


void
FreeSwapchain()
{
    u32 SwapchainImageCount = GlobalVulkan.SwapchainImageCount;

    if (SwapchainImageCount > 0)
    {
        for (u32 ImageIndex = 0;
                ImageIndex < GlobalVulkan.SwapchainImageCount;
                ++ImageIndex)
        {
            if (VK_VALID_HANDLE(GlobalVulkan.Framebuffers[ImageIndex]))
            {
                vkDestroyFramebuffer(GlobalVulkan.PrimaryDevice,GlobalVulkan.Framebuffers[ImageIndex],0);
                GlobalVulkan.Framebuffers[ImageIndex] = VK_NULL_HANDLE;
            }
            if (VK_VALID_HANDLE(GlobalVulkan.SwapchainImageViews[ImageIndex]))
            {
                vkDestroyImageView(GlobalVulkan.PrimaryDevice,GlobalVulkan.SwapchainImageViews[ImageIndex],0);
                GlobalVulkan.SwapchainImageViews[ImageIndex] = VK_NULL_HANDLE;
            }
        }

        for (i32 FrameIndex = 0;
                FrameIndex < FRAME_OVERLAP;
                ++FrameIndex)
        {
            frame_data * FrameData = GlobalVulkan.FrameData + FrameIndex;
            vkFreeCommandBuffers(GlobalVulkan.PrimaryDevice,FrameData->CommandPool, 1,&FrameData->PrimaryCommandBuffer);
        }

        if ( VK_VALID_HANDLE(GlobalVulkan.RenderPass) )
        {
            vkDestroyRenderPass(GlobalVulkan.PrimaryDevice, GlobalVulkan.RenderPass, 0);
        }


        GlobalVulkan.SwapchainImageCount = 0;
    }

}

void
VulkanDestroyDepthBuffer()
{
    if ( VK_VALID_HANDLE(GlobalVulkan.DepthBuffer.Image) )
    {
        vkDestroyImage(GlobalVulkan.PrimaryDevice,GlobalVulkan.DepthBuffer.Image,0);
    }
    if ( VK_VALID_HANDLE(GlobalVulkan.DepthBuffer.ImageView) )
    {
        vkDestroyImageView(GlobalVulkan.PrimaryDevice,GlobalVulkan.DepthBuffer.ImageView,0);
    }
    if ( VK_VALID_HANDLE(GlobalVulkan.DepthBuffer.DeviceMemory) )
    {
        vkFreeMemory(GlobalVulkan.PrimaryDevice,GlobalVulkan.DepthBuffer.DeviceMemory,0);
    }

}
i32
VulkanOnWindowResize(i32 Width,i32 Height)
{
    if (GlobalVulkan.Initialized)
    {

        VulkanWaitForDevices();

        FreeSwapchain();
        
        if (VulkanCreateSwapChain(Width, Height)) return 1;

        // If window is minimized process is halt
        if (GlobalWindowIsMinimized) return 0;

        // (fef) for each frame
        for (i32 FrameIndex = 0;
                FrameIndex < FRAME_OVERLAP;
                ++FrameIndex)
        {
            frame_data * FrameData = GlobalVulkan.FrameData + FrameIndex;

            if (VulkanCreateCommandBuffers(
                        GlobalVulkan.PrimaryDevice,
                        FrameData->CommandPool,
                        1,
                        &FrameData->PrimaryCommandBuffer)) return 1;
        }

        if (VulkanInitDefaultRenderPass()) return 1;

        VulkanDestroyDepthBuffer();

        VkExtent3D Extent3D = { GlobalVulkan.WindowExtension.width, GlobalVulkan.WindowExtension.height, 1};
        if (VulkanCreateDepthBuffer(GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice, Extent3D, &GlobalVulkan.DepthBuffer))
        {
            return 1;
        }

        if (VulkanInitFramebuffers()) return 1;

        VulkanReCreatePipelinesOnWindowResize();
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

    VkDeviceCreateInfo DeviceCreateInfo = {};
    DeviceCreateInfo.sType                       = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; // VkStructureType   sType;
    DeviceCreateInfo.pNext                       = 0;                                    // Void * pNext;
    DeviceCreateInfo.flags                       = 0;                                    // VkDeviceCreateFlags   flags;
    DeviceCreateInfo.queueCreateInfoCount        = QueuesRequired;                       // u32_t   queueCreateInfoCount;
    DeviceCreateInfo.pQueueCreateInfos           = &QueueCreateInfo[0];                  // Typedef * pQueueCreateInfos;
    // DEPRECATED
    DeviceCreateInfo.enabledLayerCount           = 0;                                    // u32_t   enabledLayerCount;
    // DEPRECATED
    DeviceCreateInfo.ppEnabledLayerNames         = 0;                                    // Pointer * ppEnabledLayerNames;
    DeviceCreateInfo.enabledExtensionCount       = TotalDeviceExtReq; // u32_t   enabledExtensionCount;
    DeviceCreateInfo.ppEnabledExtensionNames     = &DeviceExtensionsRequired[0];         // Pointer * ppEnabledExtensionNames;
    DeviceCreateInfo.pEnabledFeatures            = 0;                                    // Typedef * pEnabledFeatures;

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
    
    

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkAllocateMemory);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkFreeMemory);

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateBuffer);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyBuffer);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyImage);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkGetBufferMemoryRequirements);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkGetImageMemoryRequirements);

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkMapMemory);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkUnmapMemory);

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkBindBufferMemory);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkBindImageMemory);

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
    u32 SelectedQueueFamilyIndex = UINT32_MAX;

    for (u32 i = 0;
         i < TotalPhysicalDevices;
         ++i)
    {
        VkPhysicalDeviceProperties  PhysicalDeviceProperties;
        vkGetPhysicalDeviceProperties(PhysicalDevices[i], &PhysicalDeviceProperties);

        VkPhysicalDeviceFeatures    PhysicalDeviceFeatures;
        vkGetPhysicalDeviceFeatures(PhysicalDevices[i],   &PhysicalDeviceFeatures);

        b32 SupportsAnisotropyFilter = (PhysicalDeviceFeatures.samplerAnisotropy == VK_TRUE);

        vk_version Version = GetVkVersionFromu32(PhysicalDeviceProperties.apiVersion);

        u32 TotalQueueFamilyPropertyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevices[i],&TotalQueueFamilyPropertyCount,0);

        if (TotalQueueFamilyPropertyCount > 0 && SupportsAnisotropyFilter)
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
                vk_version DriverVersion = GetVkVersionFromu32(PhysicalDeviceProperties.driverVersion);
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


i32
VulkanGetInstance(b32 EnableValidationLayer, 
                  const char * VkKHROSSurfaceExtensionName,
                  void ** pfnOSSurface, const char * OSSurfaceFuncName,
                  PFN_vkGetInstanceProcAddr VulkanInstanceProcAddress)
{
    vkGetInstanceProcAddr = VulkanInstanceProcAddress;

    if (!vkGetInstanceProcAddr)
    {
        Log("Global Instance Proc address Function is not valid\n");
        return 1;
    }

    // VULKAN INSTANCE FUNCTIONS
    VK_GLOBAL_LEVEL_FN(vkCreateInstance);
    VK_GLOBAL_LEVEL_FN(vkEnumerateInstanceExtensionProperties);
    VK_GLOBAL_LEVEL_FN(vkEnumerateInstanceLayerProperties);

    VkApplicationInfo AppInfo = {};
    AppInfo.sType                 = VK_STRUCTURE_TYPE_APPLICATION_INFO; // VkStructureType   sType;
    AppInfo.pNext                 = 0; // Void * pNext;
    AppInfo.pApplicationName      = "Render Engine"; // Char_S * pApplicationName;
    AppInfo.applicationVersion    = RENDER_VERSION; // u32_t   applicationVersion;
    AppInfo.pEngineName           = ""; // Char_S * pEngineName;
    AppInfo.engineVersion         = ENGINE_VERSION; // u32_t   engineVersion;
    AppInfo.apiVersion            = VULKAN_API_VERSION; // u32_t   apiVersion;
    
    if (!VulkanInstanceSupportsSurface(VkKHROSSurfaceExtensionName))
    {
        Log("Instance does not supports surfaces");
        return 1;
    }

    const char * ExtensionsRequired[2] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VkKHROSSurfaceExtensionName
    };

    i32 EnabledInstanceLayerCount = 0;
    const char * AllInstanceLayers[1] = {
        "VK_LAYER_KHRONOS_validation"
    };
    b32 InstanceLayersEnabledStatus[ArrayCount(AllInstanceLayers)] = {
        EnableValidationLayer 
    };

    char * InstanceLayersToEnable[ArrayCount(AllInstanceLayers)];

    u32 InstanceLayerCount;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&InstanceLayerCount,0));

    VkLayerProperties LayerProperties[40];
    Assert(ArrayCount(LayerProperties) >= InstanceLayerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&InstanceLayerCount,&LayerProperties[0]));

    for (u32 LayerIndex = 0;
                LayerIndex < ArrayCount(InstanceLayersEnabledStatus);
                ++LayerIndex)
    {
        if ( InstanceLayersEnabledStatus[LayerIndex] )
        {
            b32 Found  = false;
            for (u32 LayerPropIndex = 0;
                        LayerPropIndex < ArrayCount(LayerProperties);
                        ++LayerPropIndex)
            {
                if (strcmp(LayerProperties[LayerPropIndex].layerName,AllInstanceLayers[LayerIndex]) == 0)
                {
                    InstanceLayersToEnable[EnabledInstanceLayerCount++] = (char *)AllInstanceLayers[LayerIndex];
                    Found = true;
                    break;
                }
            }
            if (!Found)
            {
                Log("Requested Instance Layer %s but it was not found\n.",AllInstanceLayers[LayerIndex]);
                return 1;
            }
        }
    }

    VkInstanceCreateInfo InstanceCreateInfo;
    InstanceCreateInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // VkStructureType   sType;
    InstanceCreateInfo.pNext                   = 0;                                      // Void * pNext;
    InstanceCreateInfo.flags                   = 0;                                      // VkInstanceCreateFlags   flags;
    InstanceCreateInfo.pApplicationInfo        = &AppInfo;                               // Typedef * pApplicationInfo;
    InstanceCreateInfo.enabledLayerCount       = EnabledInstanceLayerCount;              // u32_t   enabledLayerCount;
    if (EnabledInstanceLayerCount)
    {
        InstanceCreateInfo.ppEnabledLayerNames = &InstanceLayersToEnable[0];             // Pointer * ppEnabledLayerNames;
    }
    InstanceCreateInfo.enabledExtensionCount   = ArrayCount(ExtensionsRequired);         // u32_t   enabledExtensionCount;
    InstanceCreateInfo.ppEnabledExtensionNames = &ExtensionsRequired[0];                 // Pointer * ppEnabledExtensionNames;

    VK_CHECK(vkCreateInstance(&InstanceCreateInfo,0,&GlobalVulkan.Instance));

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

    // physical devices
    VK_INSTANCE_LEVEL_FN(Instance,vkEnumeratePhysicalDevices);
    VK_INSTANCE_LEVEL_FN(Instance,vkEnumerateDeviceExtensionProperties);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceProperties);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceFeatures);
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

i32
CreateDescriptorSetLayoutGlobal()
{
    VkDescriptorSetLayoutBinding SimulationBufferBinding = 
        VulkanCreateDescriptorSetLayoutBinding(0,
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
        VulkanCreateDescriptorSetLayoutBinding(0,
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
VulkanAllocateDescriptor(VkDescriptorSetLayout  * SetLayout,
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

VkWriteDescriptorSet
VulkanWriteDescriptor(u32 BindingSlot,VkDescriptorSet Set,VkDescriptorType DescriptorType, VkDescriptorBufferInfo * BufferInfo)
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

// ivp -- initialize vulkan procedure
i32
InitializeVulkan(i32 Width, i32 Height, 
                 vulkan_platform_window PlatformWindow,
                 b32 EnableValidationLayer,
                 PFN_vkGetInstanceProcAddr GetInstanceProcAddr)
{
    // during creation of vulkan instance
    // base on the OS surface extension name
    // get a void pointer to function which
    // we give back to the OS to cast and invoke
    void * pfnOSSurface = 0;

    if (VulkanGetInstance(EnableValidationLayer,
                          PlatformWindow.VkKHROSSurfaceExtensionName,
                          &pfnOSSurface,PlatformWindow.OSSurfaceFuncName,
                          GetInstanceProcAddr)) return 1;

    if (PlatformWindow.pfnVulkanCreateSurface(PlatformWindow.SurfaceData,pfnOSSurface, GlobalVulkan.Instance, &GlobalVulkan.Surface)) return 1;

    if (VulkanGetPhysicalDevice()) return 1;

    if (VulkanCreateLogicaDevice()) return 1;

    if (VulkanCreateSwapChain(Width, Height)) return 1;

    // Single uniform buffer shared by each FRAME_OVERLAP
    VkDeviceSize SimulationBufferPaddedSize = FRAME_OVERLAP * PaddedUniformBuffer(sizeof(GPUSimulationData));

    if (VulkanCreateBuffer(
                GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice, 
                SimulationBufferPaddedSize, 
                VK_SHARING_MODE_EXCLUSIVE,
                VK_MEMORY_CPU_TO_GPU_PREFERRED,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                &GlobalVulkan.SimulationBuffer
                ))
    {
        return 1;
    }

    /* INIT DESCRIPTOR SETS */
    /* Level 1 */ CreateDescriptorSetLayoutGlobal();
    /* Level 2 */ CreateDescriptorSetLayoutObjects();

    VkDescriptorPoolSize DescriptorPoolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 }
	};

	VkDescriptorPoolCreateInfo PoolInfo = {};
	PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	PoolInfo.flags = 0;
	PoolInfo.maxSets = 10;
	PoolInfo.poolSizeCount = ArrayCount(DescriptorPoolSizes);
	PoolInfo.pPoolSizes = DescriptorPoolSizes;

	vkCreateDescriptorPool(GlobalVulkan.PrimaryDevice, &PoolInfo, nullptr, &GlobalVulkan._DescriptorPool);

    AllocateFrameDataObjectsBuffer(10000);

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

        if (VulkanCreateCommandPool(
                    GlobalVulkan.PrimaryDevice,
                    GlobalVulkan.PresentationQueueFamilyIndex,
                    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                    &FrameData->CommandPool
                    )) return 1;

        if (VulkanCreateCommandBuffers(
                    GlobalVulkan.PrimaryDevice,
                    FrameData->CommandPool,
                    1,
                    &FrameData->PrimaryCommandBuffer)) return 1;

#if 0
        if (VulkanCreateBuffer(
                    GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice, 
                    sizeof(GPUSimulationData), 
                    VK_SHARING_MODE_EXCLUSIVE,
                    VK_MEMORY_CPU_TO_GPU_PREFERRED,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    &FrameData->SimulationBuffer, 
                    &FrameData->SimulationBufferDeviceMemory,
                    &FrameData->SimulationBufferAlignment)) return 1;
#endif

        // from pool, this layout, out ID
        VulkanAllocateDescriptor(&GlobalVulkan._GlobalSetLayout, GlobalVulkan._DescriptorPool, &FrameData->GlobalDescriptor);

        VkDescriptorBufferInfo BufferInfo;
        BufferInfo.buffer = GlobalVulkan.SimulationBuffer.Buffer; // VkBuffer   buffer;
        BufferInfo.offset = 0;//FrameIndex * PaddedUniformBuffer(sizeof(GPUSimulationData)); // VkDeviceSize   offset;
        BufferInfo.range  = sizeof(GPUSimulationData); // VkDeviceSize   range;

        VkWriteDescriptorSet SimulationWriteSet =
            VulkanWriteDescriptor(0,FrameData->GlobalDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, &BufferInfo);

        VulkanAllocateDescriptor(&GlobalVulkan._ObjectsSetLayout, GlobalVulkan._DescriptorPool, &FrameData->ObjectsDescriptor);

        VkDescriptorBufferInfo BufferInfoObjects;
        BufferInfoObjects.buffer = FrameData->ObjectsBuffer.Buffer; // VkBuffer   buffer;
        BufferInfoObjects.offset = 0;//FrameIndex * PaddedUniformBuffer(sizeof(GPUSimulationData)); // VkDeviceSize   offset;
        BufferInfoObjects.range  = FrameData->ObjectsBuffer.MemoryRequirements.size; // VkDeviceSize   range;

        VkWriteDescriptorSet ObjectsWriteSet =
            VulkanWriteDescriptor(0,FrameData->ObjectsDescriptor, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &BufferInfoObjects);

        VkWriteDescriptorSet WriteSets[] = 
        {
            SimulationWriteSet,
            ObjectsWriteSet
        };

        vkUpdateDescriptorSets(GlobalVulkan.PrimaryDevice, ArrayCount(WriteSets), WriteSets, 0, nullptr);
    }

    // Staging buffer commands
    if (VulkanCreateCommandPool(
                GlobalVulkan.PrimaryDevice,
                GlobalVulkan.TransferOnlyQueueFamilyIndex,
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                &GlobalVulkan.CommandPoolTransferBit
                )) return 1;

    if (VulkanCreateCommandBuffers(
                GlobalVulkan.PrimaryDevice,
                GlobalVulkan.CommandPoolTransferBit,
                1,
                &GlobalVulkan.TransferBitCommandBuffer)) return 1;

    VkExtent3D Extent3D = { GlobalVulkan.WindowExtension.width, GlobalVulkan.WindowExtension.height, 1};
    if (VulkanCreateDepthBuffer(GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice, Extent3D, &GlobalVulkan.DepthBuffer))
    {
        return 1;
    }

    if (VulkanInitDefaultRenderPass()) return 1;

    if (VulkanInitFramebuffers()) return 1;

    /* INIT PIPELINE */
    VkDescriptorSetLayout LayoutSets[] = 
    {
        GlobalVulkan._GlobalSetLayout,
        GlobalVulkan._ObjectsSetLayout
    };

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = VulkanCreatePipelineLayoutCreateInfo();

    VkPushConstantRange PushConstant;
    PushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // VkShaderStageFlags   stageFlags;
    PushConstant.offset     = 0; // u32_t   offset;
    PushConstant.size       = sizeof(mesh_push_constant); // u32_t   size;

    PipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    PipelineLayoutCreateInfo.pPushConstantRanges = &PushConstant;

    PipelineLayoutCreateInfo.setLayoutCount = ArrayCount(LayoutSets);
    PipelineLayoutCreateInfo.pSetLayouts = LayoutSets;

    VK_CHECK(vkCreatePipelineLayout(GlobalVulkan.PrimaryDevice, &PipelineLayoutCreateInfo, 0, &GlobalVulkan.PipelineLayout));

    VkDeviceSize TransferbitBufferSize = Megabytes(16);
    u32 SharedBufferFamilyIndexArray[2] = {
        GlobalVulkan.GraphicsQueueFamilyIndex,
        GlobalVulkan.TransferOnlyQueueFamilyIndex
    };
    if (VulkanCreateBuffer(
                GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice, 
                TransferbitBufferSize, VK_SHARING_MODE_CONCURRENT,VK_MEMORY_CPU_TO_GPU_HOST_VISIBLE,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                &GlobalVulkan.TransferBitBuffer,
                2,&SharedBufferFamilyIndexArray[0])) return 1;

    VkDeviceSize VertexBufferSize = Megabytes(50);
    if (VulkanCreateBuffer(
                GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice, 
                VertexBufferSize, VK_SHARING_MODE_EXCLUSIVE,VK_MEMORY_GPU,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                &GlobalVulkan.VertexBuffer))
    {
        return 1;
    }

    VkDeviceSize IndexBufferSize = Megabytes(16);
    if (VulkanCreateBuffer(
                GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice, 
                IndexBufferSize, VK_SHARING_MODE_EXCLUSIVE,VK_MEMORY_GPU,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                &GlobalVulkan.IndexBuffer)) 
    {
        return 1;
    }

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

    /* ALWAYS AFTER WAIT IDLE */
    DeleteVulkanBuffer(&GlobalVulkan.TransferBitBuffer);

    DestroyTextures();

    DeleteVulkanBuffer(&GlobalVulkan.VertexBuffer);
    DeleteVulkanBuffer(&GlobalVulkan.IndexBuffer);

    VulkanDestroyPipeline();

    if (VK_VALID_HANDLE(GlobalVulkan.PipelineLayout))
    {
        vkDestroyPipelineLayout(GlobalVulkan.PrimaryDevice,GlobalVulkan.PipelineLayout,0);
    }

    FreeSwapchain();

    
    // (fef) for each frame
    for (i32 FrameIndex = 0;
            FrameIndex < FRAME_OVERLAP;
            ++FrameIndex)
    {
        frame_data * FrameData = GlobalVulkan.FrameData + FrameIndex;
        if (VK_VALID_HANDLE(FrameData->CommandPool)) 
        {
            vkDestroyCommandPool(GlobalVulkan.PrimaryDevice,FrameData->CommandPool,0);
        }
        if (VK_VALID_HANDLE(FrameData->ImageAvailableSemaphore))
        {
            vkDestroySemaphore(GlobalVulkan.PrimaryDevice, FrameData->ImageAvailableSemaphore, 0);
        }

        if (VK_VALID_HANDLE(FrameData->RenderSemaphore))
        {
            vkDestroySemaphore(GlobalVulkan.PrimaryDevice, FrameData->RenderSemaphore, 0);
        }

        if (VK_VALID_HANDLE(FrameData->RenderFence))
        {
            vkDestroyFence(GlobalVulkan.PrimaryDevice, FrameData->RenderFence, 0);
        }
    }

    DeleteFrameDataObjectsBuffer();

    DeleteVulkanBuffer(&GlobalVulkan.SimulationBuffer);

    if (VK_VALID_HANDLE(GlobalVulkan._GlobalSetLayout))
    {
        vkDestroyDescriptorSetLayout(GlobalVulkan.PrimaryDevice,GlobalVulkan._GlobalSetLayout,0);
    }

    if (VK_VALID_HANDLE(GlobalVulkan._ObjectsSetLayout))
    {
        vkDestroyDescriptorSetLayout(GlobalVulkan.PrimaryDevice,GlobalVulkan._ObjectsSetLayout,0);
    }

    if (VK_VALID_HANDLE(GlobalVulkan._DescriptorPool))
    {
        vkDestroyDescriptorPool(GlobalVulkan.PrimaryDevice,GlobalVulkan._DescriptorPool,0);
    }

    if (VK_VALID_HANDLE(GlobalVulkan.CommandPoolTransferBit)) 
    {
        vkDestroyCommandPool(GlobalVulkan.PrimaryDevice,GlobalVulkan.CommandPoolTransferBit,0);
    }

    if ( VK_VALID_HANDLE(GlobalVulkan.Swapchain) )
    {
        vkDestroySwapchainKHR(GlobalVulkan.PrimaryDevice, GlobalVulkan.Swapchain, 0);
    }

    RenderFreeShaders();

    VulkanDestroyDepthBuffer();

    /* DEVICE DESTRUCTION */

    if (VK_VALID_HANDLE(GlobalVulkan.PrimaryDevice) )vkDestroyDevice(GlobalVulkan.PrimaryDevice,0);

    if (VK_VALID_HANDLE(GlobalVulkan.SecondaryDevice) )vkDestroyDevice(GlobalVulkan.SecondaryDevice,0);

    if (VK_VALID_HANDLE(GlobalVulkan.Surface) )vkDestroySurfaceKHR(GlobalVulkan.Instance, GlobalVulkan.Surface, 0);

    if (VK_VALID_HANDLE(GlobalVulkan.Instance) )vkDestroyInstance(GlobalVulkan.Instance,0);
}
