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

enum gpu_arena_type
{
    gpu_arena_type_buffer,
    gpu_arena_type_image
};

struct gpu_arena
{
    void * Owner;
    i32 MemoryIndexType;
    VkPhysicalDevice GPU;
    VkDevice Device;

    u32 MaxSize;
    u32 CurrentSize;
    u32 Alignment;

    gpu_arena_type Type;
    void * WriteToAddr;

    VkBuffer Buffer;

#define IMAGES_PER_ARENA 256
    // this is only useful for images
    // where we need to bind to the memory
    // starting at the offset
    u32 DeviceBindingOffsetBegin;
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
    u32 Size;
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
    u32 Instance;
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
    mesh_group * MeshGroup;
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

    mesh_group * Sphere;
#if DEBUG
    i32 DebugWireframeMaterialPipelineIndex;
    void * DebugBoundingBoxVertexBuffer;
    u32 DebugBoundingBoxVertexBufferSize;
    render_units UnitsBBV;
#endif

    render_units UnitsOpaque;
    render_units UnitsTransparent;
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
    int ImageIndex;
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

    const char * MemoryArenasLabel;
    gpu_arena MemoryArenas[10];
    u32 MemoryArenaCount;

    gpu_arena * PrimaryDepthBufferArena;
    depth_buffer * PrimaryDepthBuffer;

    // OIT_WEIGHTED
    gpu_arena * WeightedColorArena;
    vulkan_image * WeightedColorImage;
    gpu_arena * WeightedRevealArena;
    vulkan_image * WeightedRevealImage;
    VkDescriptorSet _oit_WeightedColorSet;
    VkDescriptorSet _oit_WeightedRevealSet;

    b32 TextureArenaDirty;
    gpu_arena * TextureArena;

    u32  GraphicsQueueFamilyIndex;
    VkQueue GraphicsQueue;
    u32  PresentationQueueFamilyIndex;
    VkQueue PresentationQueue;
    u32  TransferOnlyQueueFamilyIndex;
    VkQueue TransferOnlyQueue;

    const char * StagingBufferLabel;
    VkCommandPool   CommandPoolTransferBit;
    VkCommandBuffer TransferBitCommandBuffer;

    gpu_arena * TransferBitArena;

    VkSampler     TextureSampler;
    VkDescriptorImageInfo ImageInfoArray[GPU_TEXTURES_ARRAY_COUNT];

    gpu_arena * VertexArena;
    gpu_arena * IndexArena;

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

    gpu_arena * SimulationArena;

    const char * FrameDataLabel;
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

    const char * PipelinesLabel;
    VkPipeline      Pipelines[MAX_PIPELINES];
    vulkan_pipeline PipelinesDefinition[MAX_PIPELINES];
    u32          PipelinesCount;

    const char * ShaderModulesLabel;
    VkShaderModule ShaderModules[16];
    u32         ShaderModulesCount;

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
CreateShaderModule(void * Buffer, size_t Size);


i32 
PushTextureData(void * Data, u32 Width, u32 Height, u32 Channels);
i32
RenderBindTexture(u32 ImageIndex);
i32
RenderDrawObjectNTimes(u32 VertexSize, u32 NTimes, u32 FirstInstance);

#endif
