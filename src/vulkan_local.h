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

extern PFN_vkGetInstanceProcAddr                     vkGetInstanceProcAddr;

// INSTANCE VULKAN
extern PFN_vkCreateInstance                          vkCreateInstance;
extern PFN_vkEnumerateInstanceExtensionProperties    vkEnumerateInstanceExtensionProperties;
extern PFN_vkEnumerateInstanceLayerProperties        vkEnumerateInstanceLayerProperties;


// PHYSICAL DEVICES
// SWAP CHAIN API
extern PFN_vkDestroySurfaceKHR                       vkDestroySurfaceKHR;
extern PFN_vkGetPhysicalDeviceSurfaceSupportKHR      vkGetPhysicalDeviceSurfaceSupportKHR;
extern PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
extern PFN_vkGetPhysicalDeviceSurfaceFormatsKHR      vkGetPhysicalDeviceSurfaceFormatsKHR;
extern PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
extern PFN_vkGetPhysicalDeviceMemoryProperties       vkGetPhysicalDeviceMemoryProperties;
extern PFN_vkGetPhysicalDeviceFormatProperties       vkGetPhysicalDeviceFormatProperties;

extern PFN_vkCreateSwapchainKHR                      vkCreateSwapchainKHR;
extern PFN_vkDestroySwapchainKHR                     vkDestroySwapchainKHR;
extern PFN_vkGetSwapchainImagesKHR                   vkGetSwapchainImagesKHR;
extern PFN_vkAcquireNextImageKHR                     vkAcquireNextImageKHR;
extern PFN_vkQueuePresentKHR                         vkQueuePresentKHR;


extern PFN_vkEnumeratePhysicalDevices                vkEnumeratePhysicalDevices;
extern PFN_vkEnumerateDeviceExtensionProperties      vkEnumerateDeviceExtensionProperties;
extern PFN_vkGetPhysicalDeviceProperties             vkGetPhysicalDeviceProperties;
extern PFN_vkGetPhysicalDeviceFeatures               vkGetPhysicalDeviceFeatures;
extern PFN_vkGetPhysicalDeviceQueueFamilyProperties  vkGetPhysicalDeviceQueueFamilyProperties;
extern PFN_vkCreateDevice                            vkCreateDevice;
extern PFN_vkGetDeviceProcAddr                       vkGetDeviceProcAddr;
extern PFN_vkDestroyInstance                         vkDestroyInstance;
extern PFN_vkCreateCommandPool                       vkCreateCommandPool;
extern PFN_vkDestroyCommandPool                      vkDestroyCommandPool;
extern PFN_vkResetCommandPool                        vkResetCommandPool;
extern PFN_vkResetCommandBuffer                      vkResetCommandBuffer;
extern PFN_vkAllocateCommandBuffers                  vkAllocateCommandBuffers;  
extern PFN_vkFreeCommandBuffers                      vkFreeCommandBuffers;  
extern PFN_vkBeginCommandBuffer                      vkBeginCommandBuffer;
extern PFN_vkEndCommandBuffer                        vkEndCommandBuffer;
extern PFN_vkCmdPushConstants                        vkCmdPushConstants;
extern PFN_vkCmdCopyBuffer                           vkCmdCopyBuffer;
extern PFN_vkCmdCopyBufferToImage                    vkCmdCopyBufferToImage;
extern PFN_vkCmdBeginRenderPass                      vkCmdBeginRenderPass;
extern PFN_vkCmdEndRenderPass                        vkCmdEndRenderPass;
extern PFN_vkCmdPipelineBarrier                      vkCmdPipelineBarrier;
extern PFN_vkCmdClearColorImage                      vkCmdClearColorImage;
extern PFN_vkCmdBindPipeline                         vkCmdBindPipeline;
extern PFN_vkCmdDraw                                 vkCmdDraw;
extern PFN_vkCmdDrawIndexed                          vkCmdDrawIndexed;
extern PFN_vkCmdBindVertexBuffers                    vkCmdBindVertexBuffers;
extern PFN_vkCmdBindIndexBuffer                      vkCmdBindIndexBuffer;
extern PFN_vkQueueSubmit                             vkQueueSubmit;
extern PFN_vkQueueWaitIdle                           vkQueueWaitIdle;
extern PFN_vkCreateShaderModule                      vkCreateShaderModule;
extern PFN_vkDestroyShaderModule                     vkDestroyShaderModule;

// LOGICAL DEVICE
extern PFN_vkGetDeviceQueue                          vkGetDeviceQueue;
extern PFN_vkDestroyDevice                           vkDestroyDevice;
extern PFN_vkDeviceWaitIdle                          vkDeviceWaitIdle;
extern PFN_vkCreateFence                             vkCreateFence;
extern PFN_vkDestroyFence                            vkDestroyFence;
extern PFN_vkWaitForFences                           vkWaitForFences;
extern PFN_vkResetFences                             vkResetFences;
extern PFN_vkCreateSemaphore                         vkCreateSemaphore;
extern PFN_vkDestroySemaphore                        vkDestroySemaphore;
extern PFN_vkCreateRenderPass                        vkCreateRenderPass;
extern PFN_vkDestroyRenderPass                       vkDestroyRenderPass;
extern PFN_vkCreateImageView                         vkCreateImageView;
extern PFN_vkCreateImage                             vkCreateImage;
extern PFN_vkCreateSampler                           vkCreateSampler;
extern PFN_vkDestroySampler                          vkDestroySampler;
extern PFN_vkDestroyImageView                        vkDestroyImageView;
extern PFN_vkCreateFramebuffer                       vkCreateFramebuffer; 
extern PFN_vkDestroyFramebuffer                      vkDestroyFramebuffer; 
extern PFN_vkCreateGraphicsPipelines                 vkCreateGraphicsPipelines;
extern PFN_vkCreatePipelineLayout                    vkCreatePipelineLayout;
extern PFN_vkDestroyPipeline                         vkDestroyPipeline;
extern PFN_vkDestroyPipelineLayout                   vkDestroyPipelineLayout;

extern PFN_vkCreateDescriptorSetLayout               vkCreateDescriptorSetLayout;
extern PFN_vkDestroyDescriptorSetLayout              vkDestroyDescriptorSetLayout;
extern PFN_vkAllocateDescriptorSets                  vkAllocateDescriptorSets;  
extern PFN_vkUpdateDescriptorSets                    vkUpdateDescriptorSets;
extern PFN_vkCreateDescriptorPool                    vkCreateDescriptorPool;
extern PFN_vkDestroyDescriptorPool                   vkDestroyDescriptorPool;
extern PFN_vkCmdBindDescriptorSets                   vkCmdBindDescriptorSets;

extern PFN_vkAllocateMemory                          vkAllocateMemory;
extern PFN_vkFreeMemory                              vkFreeMemory;
extern PFN_vkCreateBuffer                            vkCreateBuffer;
extern PFN_vkDestroyBuffer                           vkDestroyBuffer;
extern PFN_vkDestroyImage                            vkDestroyImage;
extern PFN_vkGetBufferMemoryRequirements             vkGetBufferMemoryRequirements;
extern PFN_vkGetImageMemoryRequirements              vkGetImageMemoryRequirements;
extern PFN_vkMapMemory                               vkMapMemory;
extern PFN_vkUnmapMemory                             vkUnmapMemory;
extern PFN_vkBindBufferMemory                        vkBindBufferMemory;
extern PFN_vkBindImageMemory                         vkBindImageMemory;


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

enum vulkan_destructor_type
{
    vulkan_destructor_type_vkDestroySurfaceKHR = 0,
    vulkan_destructor_type_vkDestroySwapchainKHR,
    vulkan_destructor_type_vkDestroyInstance,
    vulkan_destructor_type_vkDestroyCommandPool,
    vulkan_destructor_type_vkDestroyShaderModule,
    vulkan_destructor_type_vkDestroyDevice,
    vulkan_destructor_type_vkDestroyFence,
    vulkan_destructor_type_vkDestroySemaphore,
    vulkan_destructor_type_vkDestroyRenderPass,
    vulkan_destructor_type_vkDestroySampler,
    vulkan_destructor_type_vkDestroyImageView,
    vulkan_destructor_type_vkDestroyFramebuffer ,
    vulkan_destructor_type_vkDestroyPipeline,
    vulkan_destructor_type_vkDestroyPipelineLayout,
    vulkan_destructor_type_vkDestroyDescriptorSetLayout,
    vulkan_destructor_type_vkDestroyDescriptorPool,
    vulkan_destructor_type_vkDestroyBuffer,
    vulkan_destructor_type_vkDestroyImage,

    // CUSTOM DESTRUCTORS
    vulkan_destructor_type_vkDestroyBufferCustom,
    vulkan_destructor_type_vkDestroyImageCustom
};

struct vulkan_destroy_queue_item
{
    vulkan_destructor_type DestructorType;
    void * Params[5];
    u32 CountParams;
};
struct vulkan_destroy_queue
{
    vulkan_destroy_queue_item Items[100];
    u32 ItemsCount;
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
    VkPipeline Pipeline;
};

struct vulkan_buffer
{
    VkBuffer             Buffer;
    VkDeviceMemory       DeviceMemory;
    VkMemoryRequirements MemoryRequirements;
    VkDevice             DeviceAllocator;
};

struct vulkan_image
{
    VkImage              Image;
    VkImageView          ImageView;
    VkFormat             Format;
    VkDeviceMemory       DeviceMemory;
    VkMemoryRequirements MemoryRequirements;
    VkDevice             AllocatorDevice;
};

typedef vulkan_image depth_buffer ;

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

struct ground_density_volume_pass
{
    i32 x,y,z;
    VkFramebuffer Framebuffer;
    vulkan_image Color; 
    VkRenderPass Renderpass;
    VkSampler Sampler;
    VkDescriptorImageInfo Descriptor;
};

struct ground_info
{
    VkCommandPool   CommandPool;
    VkCommandBuffer CommandBuffer[6];

    vulkan_buffer VertexBuffer;

    VkDescriptorPool DescriptorPool;

    VkDescriptorSetLayout SetLayoutGlobals;
    VkDescriptorSet DescriptorGlobals;
    vulkan_buffer BufferGlobals;

    VkDescriptorSetLayout SetLayoutDensityVolume;

    VkDescriptorSetLayout SetLayoutChunks;
    VkDescriptorSet DescriptorChunks;
    vulkan_buffer BufferChunks;
    u32 MaxChunks;

    vulkan_pipeline PipelineBuildDensities;
    VkPipelineLayout PipelineLayoutBuildDensities;
    VkShaderModule ShaderVertexBuildDensity;
    VkShaderModule ShaderGeometryBuildDensity;
    VkShaderModule ShaderFragmentBuildDensity;

    ground_density_volume_pass DensityVolumePass;
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

    vulkan_image  TextureImage;
    VkSampler     TextureSampler;

    vulkan_buffer VertexBuffer;
    vulkan_buffer IndexBuffer;

    VkRenderPass  RenderPass;
    VkFramebuffer Framebuffers[3];

    VkDescriptorSetLayout _GlobalSetLayout;
    VkDescriptorSetLayout _ObjectsSetLayout;
    VkDescriptorSetLayout _DebugTextureSetLayout;
    VkDescriptorSet       _DebugTextureSet;
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

    VkPipelineLayout PipelineLayout[2];
    VkPipelineLayout CurrentPipelineLayout;


    VkPipeline      Pipelines[2];
    vulkan_pipeline PipelinesDefinition[2];
    u32          PipelinesCount;

    VkShaderModule ShaderModules[4];
    u32         ShaderModulesCount;

    ground_info GroundInfo;
};

struct mesh_push_constant
{
    m4 Model;
    v4 DebugColor;
};


extern vulkan     GlobalVulkan;
extern b32        GlobalWindowIsMinimized;

VkDescriptorSetLayoutBinding
VH_CreateDescriptorSetLayoutBinding(u32 BindingSlot,VkDescriptorType DescriptorType,VkShaderStageFlags ShaderStageFlags);

b32
VH_GetSupportedDepthFormat(VkPhysicalDevice PhysicalDevice, VkFormat *DepthFormat);

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

inline frame_data *
GetCurrentFrame()
{
    frame_data * FrameData = GlobalVulkan.FrameData + (GlobalVulkan._CurrentFrameData % FRAME_OVERLAP);
    
    return FrameData;
}


i32
VH_FindSuitableMemoryIndex(VkPhysicalDevice PhysicalDevice, VkMemoryRequirements MemoryRequirements,VkMemoryPropertyFlags PropertyFlags);

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
VH_PipelineBuilder(vulkan_pipeline * VulkanPipeline,VkDevice Device, VkRenderPass RenderPass);

i32
VH_CreateCommandBuffers(VkDevice Device,VkCommandPool CommandPool,u32 CommandBufferCount,VkCommandBuffer * CommandBuffers);

i32
VH_CreateCommandPool(VkDevice Device, u32 QueueFamilyIndex, VkCommandPoolCreateFlags CommandPoolCreateFlags, VkCommandPool * CommandPool);

i32
VH_CreateDepthBuffer(VkPhysicalDevice PhysicalDevice,VkDevice Device, VkExtent3D Extent, depth_buffer * DepthBuffer);

VkWriteDescriptorSet
VH_WriteDescriptor(u32 BindingSlot,VkDescriptorSet Set,VkDescriptorType DescriptorType, VkDescriptorBufferInfo * BufferInfo);

i32
VH_AllocateDescriptor(VkDescriptorSetLayout  * SetLayout, VkDescriptorPool Pool, VkDescriptorSet * Set);

i32
VH_CopyBuffer(VkCommandBuffer CommandBuffer, VkBuffer Src, VkBuffer Dest, VkDeviceSize Size, VkDeviceSize Offset);


i32
VH_BeginCommandBuffer(VkCommandBuffer CommandBuffer);
i32
VH_EndCommandBuffer(VkCommandBuffer CommandBuffer, VkQueue FamilyQueue);

void
VH_DestroyImage(vulkan_image * Image);

i32
VH_PushVertexData(void * Data, u32 DataSize, u32 BaseOffset);


#endif
