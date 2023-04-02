/*
 * - Initialize library pfn (AKA Instance)
 * - Create debug callback if errors
 * - Create surface
 * - Pick physical device
 * - Create Logical device for physical device
 * - Create swapchain & swapchain views
 * - Create all unalloc buffers (wrapped in arenas)
 *   This step is to understand memory req
 *
 * - Allocate device mem pools
 * - Define Descriptor set LAYOUTS
 * - Create descriptor pool
 * - Create frames command pools/barriers
 * - Create Pipeline Layouts
 * - Create staging command buffers
 * - Create depth buffer/color images
 * - Transition color images to present
 * - Create render passes
 * - Create frame buffers
 *   This is delayed as we need depth buffer created
 *   Otherwise should go with swap chain stuff
 *
 * - Create sampler for all textures
 * - Update descriptor set for OIT images
 *   Can be done earlier but for I do it 
 *   at the end to have clear code
 *
 *
 *
 *   FRAME data 
 *   buffers (arena) and  descriptor sets
 *   are depedant but is not handle in code
 *   if you destroy arenas (dont know the case yet)
 *   you will need to update frame data descriptors.
 *   Meaning bind to the new memory
 *
 */

#ifndef VULKAN_INITIALIZER_H
#define VULKAN_INITIALIZER_H

#include "hierarchy_tree.h"
#include "game_math.h"
#include "game_assets.h"
#include "gpu_heap.h"
#include "hash_table.h"

#define MAX_PIPELINES 100
#define GPU_TEXTURES_ARRAY_COUNT 10

#define VK_FAILS(result) (result != VK_SUCCESS)
#define VK_PASSES(result) (result == VK_SUCCESS)
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
 * - 1.3.+ query image requirements without creating images
 */
#define VULKAN_API_VERSION  VK_MAKE_API_VERSION(0,1,3,0)

#define VK_MEMORY_GPU                       (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
#define VK_MEMORY_CPU_TO_GPU_PREFERRED      (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
#define VK_MEMORY_CPU_TO_GPU_HOST_VISIBLE   (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
#define VK_MEMORY_LARGE_TARGETS_NOT_STORED  (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)

// vulkan is verbose
// all func calls that return VkResult follow this logic
#define VK_CHECK(FunCall) \
    if (VK_FAILS(FunCall)) \
    { \
        Log("Error during function %s\n",#FunCall); \
        return 1; \
    } \

#if 1
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
extern PFN_vkGetPhysicalDeviceFeatures2              vkGetPhysicalDeviceFeatures2;
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
extern PFN_vkCreateDebugUtilsMessengerEXT            vkCreateDebugUtilsMessengerEXT;
extern PFN_vkDestroyDebugUtilsMessengerEXT           vkDestroyDebugUtilsMessengerEXT;
extern PFN_vkSetDebugUtilsObjectNameEXT              vkSetDebugUtilsObjectNameEXT;

//  LOGICAL DEVICE
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
extern PFN_vkCmdSetViewport                          vkCmdSetViewport;
extern PFN_vkCmdSetScissor                           vkCmdSetScissor;

extern PFN_vkAllocateMemory                          vkAllocateMemory;
extern PFN_vkFreeMemory                              vkFreeMemory;
extern PFN_vkCreateBuffer                            vkCreateBuffer;
extern PFN_vkDestroyBuffer                           vkDestroyBuffer;
extern PFN_vkDestroyImage                            vkDestroyImage;
extern PFN_vkGetBufferMemoryRequirements             vkGetBufferMemoryRequirements;
extern PFN_vkGetDeviceImageMemoryRequirements        vkGetDeviceImageMemoryRequirements;
extern PFN_vkGetImageMemoryRequirements              vkGetImageMemoryRequirements;
extern PFN_vkMapMemory                               vkMapMemory;
extern PFN_vkUnmapMemory                             vkUnmapMemory;
extern PFN_vkBindBufferMemory                        vkBindBufferMemory;
extern PFN_vkBindImageMemory                         vkBindImageMemory;

extern PFN_vkCmdSetDepthTestEnable                   vkCmdSetDepthTestEnable;
extern PFN_vkCmdNextSubpass                          vkCmdNextSubpass;

#endif

struct vk_version
{
    u32 Major,Minor,Patch;
};

enum vulkan_destructor_type
{
    vulkan_destructor_type_vkDestroyUnknown = 0,
    vulkan_destructor_type_vkDestroySurfaceKHR,
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
    vulkan_destructor_type_vkDestroyDebugUtilsMessengerEXT,
    vulkan_destructor_type_vkFreeCommandBuffers,

    // CUSTOM DESTRUCTORS
    vulkan_destructor_type_vkDestroyArenaCustom,
    vulkan_destructor_type_vkDestroyMemoryPoolCustom,
    vulkan_destructor_type_vkDestroyHeapCustom,

    vulkan_destructortype_LAST
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
    VkPipelineColorBlendAttachmentState ColorBlendAttachment[2];
    u32 ColorBlendAttachmentCount;
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

typedef vulkan_image depth_buffer ;

#if 0
struct gpu_arena
{
    i32 MemoryIndexType;
    u32 MaxSize;
    u32 CurrentSize;
    u32 Alignment;
    VkDeviceMemory DeviceMemory;
    VkDevice Device;
};
#else

struct gpu_arena
{
    // For hierarchy tree 
    // This is a pointer to the struct vulkan arena pointer
    void * Owner;

    i32                MemoryIndexType;
    VkPhysicalDevice   GPU;
    VkDevice           Device;

    u64 MaxSize;
    u64 CurrentSize;
    u32 Alignment;

    gpu_arena_type Type;
    void * WriteToAddr;

    VkBuffer Buffer;

#define IMAGES_PER_ARENA 256
    // this is only useful for images
    // where we need to bind to the memory
    // starting at the offset
    u64 DeviceBindingOffsetBegin;
    u32 ImageCount;
    vulkan_image Images[256];
};
#endif

struct frame_data 
{
    VkFence     RenderFence;
    VkSemaphore ImageAvailableSemaphore;
    VkSemaphore RenderSemaphore;

    VkCommandPool   CommandPool;
    VkCommandBuffer PrimaryCommandBuffer;

    VkDescriptorSet GlobalDescriptor;
    VkDescriptorSet ObjectsDescriptor;

    gpu_arena * ObjectsArena;
    u32 ObjectsCount;
};

#define MAX_GPU_FRAME_ENTITIES 1000
struct vulkan_create_instance
{
    VkApplicationInfo AppInfo;
    VkInstanceCreateInfo CreateInfo;
    u32 NumberOfExtensions;
    char ** ExtensionsRequired;
    u32 NumberOfInstanceLayers;
    char ** InstanceLayers;
};

struct vulkan_node
{
    void * ID;
    void * Owner;
    vulkan_destructor_type DestructorType;
    void * Data;
};

#if DEBUG
struct vulkan_obj_name
{
    b32 InUse;
    char Name[40];
};

struct vulkan_debug_obj_name_cache
{
    vulkan_obj_name Objects[256];
    u32 Count;
};
#endif

// number of frames  [0-2] we are looping
#define FRAME_OVERLAP 2


struct device_memory_pool
{
    VkDevice Device;
    VkDeviceMemory DeviceMemory;
    u64 Size;
};
struct device_memory_pools
{
    device_memory_pool DeviceMemoryPool[VK_MAX_MEMORY_TYPES];
    u32 Count = VK_MAX_MEMORY_TYPES;
};


struct mesh_push_constant
{
    v4 DebugColor;
    int ImageIndex;
};


struct gpu_memory_mapping_result
{
    void * BeginAddress;
    u64 Instance;
    b32 Success;
};

struct graphics_platform_window
{
    vulkan_create_surface * pfnVulkanCreateSurface;
    void *                  SurfaceData;
    const char *            VkKHROSSurfaceExtensionName;
    const char *            OSSurfaceFuncName;
};

struct graphics_platform_window_result
{
    b32 Success;
    graphics_platform_window Window;
};


enum polygon_mode
{
    polygon_mode_fill,
    polygon_mode_line
};

struct render_unit
{
    m4 ModelTransform;
    v4 Color;
    i32 TextureID;
    u32 MaterialPipelineIndex[2];
};

struct render_units
{
    render_unit  * Units;
    u32 UnitsLimit;
    u32 UnitsCount;
};

struct render_controller
{
    v3 WorldUp;
    m4 ViewMoveMatrix;
    m4 ViewRotationMatrix;
    m4 ViewTransform;
    m4 Projection;

    u32 ScreenWidth;
    u32 ScreenHeight;
    r32 WidthOverHeight;
    r32 OneOverWidthOverHeight;
};

struct GPUSimulationData
{
    v4 AmbientLight;
    v4 SunlightDirection;
    v4 SunlightColor;
};

struct GPUObjectData
{
    m4 MVP;
    m4 ModelMatrix;
    m4 ViewMatrix;
    v4 Color;
    alignas(16) v2 TextSample;
    alignas(4) int ImageIndex;
};

struct vulkan_hash_table_entry
{
    gpu_heap_block * Block;
    u32 AssetID;
};

struct vulkan
{
    b32 Initialized;
    memory_arena InitArena;
    hierarchy_tree * HTree;

    VkInstance       Instance;

    // Primary GPU
    VkPhysicalDevice PrimaryGPU;
    VkDevice         PrimaryDevice;

    const char * DeviceMemoryPoolsLabel;
    device_memory_pools DeviceMemoryPools;

    // Hash Table for heap assets
    hash_table HeapAssetsHashTable;

    // Nameless Heaps
    const char * MemoryHeapLabel;
    gpu_heap HeapAlloc[10];
    u32 HeapAllocCount;

    // Friendly heap pointer
    gpu_heap * TextureHeap;
    gpu_heap * VertexHeap;
    gpu_heap * IndexHeap;
    b32        TextureArenaDirty;

    // Nameless Arenas
    const char * MemoryArenasLabel;
    gpu_arena MemoryArenas[10];
    u32 MemoryArenaCount;

    // Friendly arena pointer
    gpu_arena    * PrimaryDepthBufferArena;
    depth_buffer * PrimaryDepthBuffer;
    gpu_arena    * TransferBitArena;
    gpu_arena    * SimulationArena;
#if 0 // heap now
    gpu_arena * TextureArena;
    gpu_arena * VertexArena;
    gpu_arena * IndexArena;
#endif


    // OIT_WEIGHTED
    gpu_arena       * WeightedColorArena;
    vulkan_image    * WeightedColorImage;
    gpu_arena       * WeightedRevealArena;
    vulkan_image    * WeightedRevealImage;
    VkDescriptorSet   _oit_WeightedColorSet;
    VkDescriptorSet   _oit_WeightedRevealSet;

    u32       GraphicsQueueFamilyIndex;
    VkQueue   GraphicsQueue;
    u32       PresentationQueueFamilyIndex;
    VkQueue   PresentationQueue;
    u32       TransferOnlyQueueFamilyIndex;
    VkQueue   TransferOnlyQueue;

    const char        * StagingBufferLabel;
    VkCommandPool     CommandPoolTransferBit;
    VkCommandBuffer   TransferBitCommandBuffer;

    VkSampler     TextureSampler;
    VkDescriptorImageInfo ImageInfoArray[GPU_TEXTURES_ARRAY_COUNT];

    const char * RenderPassLabel;
    VkRenderPass  RenderPass;
    VkRenderPass  RenderPassTransparency;
    VkFramebuffer Framebuffers[3];
    VkFramebuffer FramebuffersTransparency[3];

    const char * DescriptorSetLayoutLabel;
    VkDescriptorSetLayout _GlobalSetLayout;
    VkDescriptorSetLayout _ObjectsSetLayout;
    VkDescriptorSetLayout _DebugTextureSetLayout;
    VkDescriptorSetLayout _oit_WeightedColorAttachmentInputsSetLayout;
    VkDescriptorSetLayout _oit_WeightedRevealAttachmentInputsSetLayout;
    VkDescriptorSet       _DebugTextureSet;
    VkDescriptorPool _DescriptorPool;

    const char * FrameDataLabel;
    frame_data FrameData[FRAME_OVERLAP];
    i32 _CurrentFrameData;

    // Secondary GPU if available
    VkPhysicalDevice SecondaryGPU;
    VkDevice         SecondaryDevice;

    // Presentation
    VkSurfaceKHR     Surface;
    VkSwapchainKHR   Swapchain;
    VkFormat         SwapchainImageFormat;
    VkExtent2D       WindowExtension;
    VkImage          SwapchainImages[3];
    VkImageView      SwapchainImageViews[3];
    u32              SwapchainImageCount;
    u32              CurrentSwapchainImageIndex;

    VkPipelineLayout PipelineLayout[2];
    VkPipelineLayout CurrentPipelineLayout;

    const char        * PipelinesLabel;
    VkPipeline        Pipelines[MAX_PIPELINES];
    vulkan_pipeline   PipelinesDefinition[MAX_PIPELINES];
    u32               PipelinesCount;

    const char       * ShaderModulesLabel;
    VkShaderModule   ShaderModules[16];
    u32              ShaderModulesCount;

#if DEBUG
    VkDebugUtilsMessengerEXT DefaultDebugCb;
#endif 
};


i32
InitializeVulkan(i32 Width, i32 Height, 
                 graphics_platform_window PlatformWindow,
                 b32 EnableValidationLayer,
                 PFN_vkGetInstanceProcAddr GetInstanceProcAddr);

void
CloseVulkan();

i32
OnWindowResize(i32 Width,i32 Height);
i32 
WaitForRender();
i32
RenderBeginPass(v4 ClearColor);
i32
EndRenderPass();

i32
CreateShaderModule(void * Buffer, size_t Size);
i32 
PushTextureData(void * Data, u32 Width, u32 Height, u32 Channels);
i32
RenderBindTexture(u32 ImageIndex);
i32
RenderDrawObjectNTimes(u32 VertexSize, u32 NTimes, u32 FirstInstance);

i32
VH_FindSuitableMemoryIndex(VkPhysicalDevice PhysicalDevice, VkMemoryRequirements MemoryRequirements,VkMemoryPropertyFlags PropertyFlags);
i32
VH_FindSuitableMemoryIndex(VkPhysicalDevice PhysicalDevice, VkMemoryRequirements2 MemoryRequirements,VkMemoryPropertyFlags PropertyFlags);

#endif
