/*
 * This library is meant to be OS independent
 * All OS calls are perform in ???_platform.cpp OS .exe
 *  - Library load and Instance proc address is handle
 *  - OS specific window creation
 * InitializeVulkan will receive opaque pointers to
 * the function/data which is to be called in order to
 * create OS specific window
 *
 */

#include "render.h"

#define VK_FAILS(result) (result != VK_SUCCESS)
#define VK_SUCCESS(result) (result == VK_SUCCESS)
#define VK_VALID_HANDLE(handle) (handle != VK_NULL_HANDLE)
#define VK_INVALID_HANDLE(handle) (handle == VK_NULL_HANDLE)

#define VK_GLOBAL_LEVEL_FN( fun ) fun = (PFN_##fun)vkGetInstanceProcAddr(0,#fun)
// watch out for the convention name of instance as "Instance"
#define VK_INSTANCE_LEVEL_FN(Instance, fun ) fun = (PFN_##fun)vkGetInstanceProcAddr(Instance,#fun)
#define VK_DEVICE_LEVEL_FN(Device, fun )   fun = (PFN_##fun)vkGetDeviceProcAddr(Device,#fun)

#define RENDER_VERSION      VK_MAKE_VERSION(1,0,0)
#define ENGINE_VERSION      VK_MAKE_VERSION(1,0,0)
#define VULKAN_API_VERSION  VK_MAKE_VERSION(1,0,0)

struct vk_version
{
    uint32 Major,Minor,Patch;
};

struct vulkan_pipeline
{
    VkPipelineShaderStageCreateInfo ShaderStages[3];
    uint32                          ShaderStagesCount;

    VkPipelineVertexInputStateCreateInfo   VertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo InputAssembly;

    VkViewport Viewport;
    VkRect2D   Scissor;

    VkPipelineRasterizationStateCreateInfo Rasterizer;
    VkPipelineColorBlendAttachmentState    ColorBlendAttachment;
    VkPipelineMultisampleStateCreateInfo   Multisampling;

    VkPipelineLayout PipelineLayout;
};

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

VkPipelineVertexInputStateCreateInfo
VulkanCreateVertexInputStateInfo()
{
    VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo;
    PipelineVertexInputStateCreateInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO; // VkStructureType   sType;
    PipelineVertexInputStateCreateInfo.pNext                           = 0; // Void * pNext;
    PipelineVertexInputStateCreateInfo.flags                           = 0; // VkPipelineVertexInputStateCreateFlags   flags;
    PipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount   = 0; // uint32_t   vertexBindingDescriptionCount;
    PipelineVertexInputStateCreateInfo.pVertexBindingDescriptions      = 0; // Typedef * pVertexBindingDescriptions;
    PipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0; // uint32_t   vertexAttributeDescriptionCount;
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

    PipelineColorBlendAttachmentState.blendEnable         = VK_FALSE; // VkBool32   blendEnable;
#if 0
    PipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_; // VkBlendFactor   srcColorBlendFactor;
    PipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_; // VkBlendFactor   dstColorBlendFactor;
    PipelineColorBlendAttachmentState.colorBlendOp        = VK_BLEND_OP_; // VkBlendOp   colorBlendOp;
    PipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_; // VkBlendFactor   srcAlphaBlendFactor;
    PipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_; // VkBlendFactor   dstAlphaBlendFactor;
    PipelineColorBlendAttachmentState.alphaBlendOp        = VK_BLEND_OP_; // VkBlendOp   alphaBlendOp;
#endif
    PipelineColorBlendAttachmentState.colorWriteMask      = 
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // VkColorComponentFlags   colorWriteMask;

    return PipelineColorBlendAttachmentState;
};

struct vulkan
{
    bool32 Initialized;

    VkInstance       Instance;

    // Primary GPU
    VkPhysicalDevice PrimaryGPU;
    VkDevice         PrimaryDevice;

    uint32  GraphicsQueueFamilyIndex;
    VkQueue GraphicsQueue;
    uint32  PresentationQueueFamilyIndex;
    VkQueue PresentationQueue;

    VkCommandPool   CommandPool;
    VkCommandBuffer PrimaryCommandBuffer[3];

    VkRenderPass  RenderPass;
    VkFramebuffer Framebuffers[3];

    VkFence     RenderFence;
    VkSemaphore ImageAvailableSemaphore;
    VkSemaphore RenderSemaphore;

    VkShaderModule TriangleVertexShader;
    VkShaderModule TriangleFragmentShader;

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
    uint32         SwapchainImageCount;


    VkShaderModule ShaderModules[2];
    uint32         ShaderModulesCount;
};

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
PFN_vkCmdBeginRenderPass                      vkCmdBeginRenderPass                      = 0;
PFN_vkCmdEndRenderPass                        vkCmdEndRenderPass                        = 0;
PFN_vkCmdPipelineBarrier                      vkCmdPipelineBarrier                      = 0;
PFN_vkCmdClearColorImage                      vkCmdClearColorImage                      = 0;
PFN_vkQueueSubmit                             vkQueueSubmit                             = 0;
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
PFN_vkDestroyImageView                        vkDestroyImageView                        = 0;
PFN_vkCreateFramebuffer                       vkCreateFramebuffer                       = 0; 
PFN_vkDestroyFramebuffer                      vkDestroyFramebuffer                      = 0; 

// vulkan is verbose
// all func calls that return VkResult follow this logic
#define VK_CHECK(FunCall) \
    if (VK_FAILS(FunCall)) \
    { \
        Log("Error during function %s\n",#FunCall); \
        return 1; \
    } \


global_variable vulkan        GlobalVulkan    = {};
global_variable bool32        GlobalWindowIsMinimized = false;


struct vulkan_device_extensions
{
    const char * DeviceExtensions[1];
    uint32 Count;
};

vulkan_device_extensions
GetRequiredDeviceExtensions()
{
    vulkan_device_extensions Ext;
    Ext.DeviceExtensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME; // CONSTANTARRAY   DeviceExtensions;
    Ext.Count               = 1;                               // uint32   Count;
    return Ext;
}

struct surface_capabilities
{
    VkExtent2D                      SwapChainExtent;
    uint32                          ImageCount;
    VkImageUsageFlags               ImageUsageFlags;
    bool32                          ValidSurface;
    VkSurfaceTransformFlagBitsKHR   SurfaceTransforms;
    bool32                          Minimized;
};

int32
VulkanGetSurfaceCapabilities(VkPhysicalDevice PhysicalDevice,VkSurfaceKHR Surface,
                             uint32 Width, uint32 Height,
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

int32
GetVulkanSurfaceFormat(VkPhysicalDevice PhysicalDevice,VkSurfaceKHR Surface,VkSurfaceFormatKHR * Format)
{
    *Format = { VK_FORMAT_UNDEFINED, VK_COLORSPACE_SRGB_NONLINEAR_KHR };

    uint32 FormatsCount = 0;

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
        for (uint32 SurfaceFormatsIndex = 0;
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

int32
GetVulkanSurfacePresentMode(VkPhysicalDevice PhysicalDevice,VkSurfaceKHR Surface,VkPresentModeKHR * PresentMode)
{
    *PresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

    uint32 PresentModesCount;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModesCount,0));

    VkPresentModeKHR SurfacePresentModes[20];
    Assert(ArrayCount(SurfacePresentModes) >= PresentModesCount);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModesCount, &SurfacePresentModes[0]));

    bool32 PresentModeFound = false;
    for (uint32 PresentModeIndex = 0;
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

        for (uint32 PresentModeIndex = 0;
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

int32
VulkanCreateSwapChain(int32 Width, int32 Height)
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
    SwapChainCreateInfo.minImageCount          = Capabilities.ImageCount;                     // uint32_t   minImageCount;
    SwapChainCreateInfo.imageFormat            = Format.format;                               // VkFormat   imageFormat;
    SwapChainCreateInfo.imageColorSpace        = Format.colorSpace;                           // VkColorSpaceKHR   imageColorSpace;
    SwapChainCreateInfo.imageExtent            = Capabilities.SwapChainExtent;                // VkExtent2D   imageExtent;
    SwapChainCreateInfo.imageArrayLayers       = 1;                                           // uint32_t   imageArrayLayers;
    SwapChainCreateInfo.imageUsage             = Capabilities.ImageUsageFlags;                // VkImageUsageFlags   imageUsage;
    SwapChainCreateInfo.imageSharingMode       = VK_SHARING_MODE_EXCLUSIVE;                   // VkSharingMode   imageSharingMode;
    SwapChainCreateInfo.queueFamilyIndexCount  = 0;                                           // uint32_t   queueFamilyIndexCount;
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

    uint32 SwapchainImageCount = 0;
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
    ImageSubresourceRange.baseMipLevel   = 0; // uint32_t   baseMipLevel;
    ImageSubresourceRange.levelCount     = 1; // uint32_t   levelCount;
    ImageSubresourceRange.baseArrayLayer = 0; // uint32_t   baseArrayLayer;
    ImageSubresourceRange.layerCount     = 1; // uint32_t   layerCount;

    for (uint32 ImageIndex = 0;
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

int32
VulkanInitDefaultRenderPass()
{
    VkAttachmentDescription AttachmentDescription;
    AttachmentDescription.flags          = 0;                                 // VkAttachmentDescriptionFlags   flags;
    AttachmentDescription.format         = GlobalVulkan.SwapchainImageFormat; // VkFormat   format;
    AttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;             // VkSampleCountFlagBits   samples;
    AttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;       // VkAttachmentLoadOp   loadOp;
    AttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;      // VkAttachmentStoreOp   storeOp;
    AttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;   // VkAttachmentLoadOp   stencilLoadOp;
    AttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // VkAttachmentStoreOp   stencilStoreOp;
    AttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;         // VkImageLayout   initialLayout;
    AttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;   // VkImageLayout   finalLayout;

    VkAttachmentReference AttachmentReference;
    AttachmentReference.attachment = 0;                                        // uint32_t   attachment;
    AttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VkImageLayout   layout;

    VkSubpassDescription SubpassDescription;
    SubpassDescription.flags                   = 0;                               // VkSubpassDescriptionFlags   flags;
    SubpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS; // VkPipelineBindPoint   pipelineBindPoint;
    SubpassDescription.inputAttachmentCount    = 0;                               // uint32_t   inputAttachmentCount;
    SubpassDescription.pInputAttachments       = 0;                               // Typedef * pInputAttachments;
    SubpassDescription.colorAttachmentCount    = 1;                               // uint32_t   colorAttachmentCount;
    SubpassDescription.pColorAttachments       = &AttachmentReference;            // Typedef * pColorAttachments;
    SubpassDescription.pResolveAttachments     = 0;                               // Typedef * pResolveAttachments;
    SubpassDescription.pDepthStencilAttachment = 0;                               // Typedef * pDepthStencilAttachment;
    SubpassDescription.preserveAttachmentCount = 0;                               // uint32_t   preserveAttachmentCount;
    SubpassDescription.pPreserveAttachments    = 0;                               // Typedef * pPreserveAttachments;

    VkRenderPassCreateInfo RenderPassCreateInfo;
    RenderPassCreateInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO; // VkStructureType   sType;
    RenderPassCreateInfo.pNext           = 0;                                         // Void * pNext;
    RenderPassCreateInfo.flags           = 0;                                         // VkRenderPassCreateFlags   flags;
    RenderPassCreateInfo.attachmentCount = 1;                                         // uint32_t   attachmentCount;
    RenderPassCreateInfo.pAttachments    = &AttachmentDescription;                    // Typedef * pAttachments;
    RenderPassCreateInfo.subpassCount    = 1;                                         // uint32_t   subpassCount;
    RenderPassCreateInfo.pSubpasses      = &SubpassDescription;                       // Typedef * pSubpasses;
    RenderPassCreateInfo.dependencyCount = 0;                                         // uint32_t   dependencyCount;
    RenderPassCreateInfo.pDependencies   = 0;                                         // Typedef * pDependencies;

    VK_CHECK(vkCreateRenderPass(GlobalVulkan.PrimaryDevice,&RenderPassCreateInfo, 0, &GlobalVulkan.RenderPass));

    return 0;
}

int32
VulkanInitFramebuffers()
{

    for (uint32 ImageIndex = 0;
                ImageIndex < GlobalVulkan.SwapchainImageCount;
                ++ImageIndex)
    {
        VkFramebufferCreateInfo FramebufferCreateInfo;
        FramebufferCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;     // VkStructureType   sType;
        FramebufferCreateInfo.pNext           = 0;                                             // Void * pNext;
        FramebufferCreateInfo.flags           = 0;                                             // VkFramebufferCreateFlags   flags;
        FramebufferCreateInfo.renderPass      = GlobalVulkan.RenderPass;                       // VkRenderPass   renderPass;
        FramebufferCreateInfo.attachmentCount = 1;                                             // uint32_t   attachmentCount;
        FramebufferCreateInfo.pAttachments    = &GlobalVulkan.SwapchainImageViews[ImageIndex]; // Typedef * pAttachments;
        FramebufferCreateInfo.width           = GlobalVulkan.WindowExtension.width;            // uint32_t   width;
        FramebufferCreateInfo.height          = GlobalVulkan.WindowExtension.height;           // uint32_t   height;
        FramebufferCreateInfo.layers          = 1;                                             // uint32_t   layers;

        VK_CHECK( vkCreateFramebuffer(GlobalVulkan.PrimaryDevice, &FramebufferCreateInfo, 0, &GlobalVulkan.Framebuffers[ImageIndex]));
    }

    return 0;
}

int32
VulkanCreateCommandPool()
{
    VkCommandPoolCreateInfo CommandPoolCreateInfo;

    VkCommandPoolCreateFlags CommandPoolCreateFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    CommandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;      // VkStructureType   sType;
    CommandPoolCreateInfo.pNext            = 0;                                               // Void * pNext;
    CommandPoolCreateInfo.flags            = CommandPoolCreateFlags;                          // VkCommandPoolCreateFlags   flags;
    CommandPoolCreateInfo.queueFamilyIndex = GlobalVulkan.PresentationQueueFamilyIndex;       // uint32_t   queueFamilyIndex;

    VK_CHECK(vkCreateCommandPool(GlobalVulkan.PrimaryDevice,&CommandPoolCreateInfo,0,&GlobalVulkan.CommandPool));

    return 0;
}

int32
VulkanCreateCommandBuffers(VkDevice Device,VkCommandPool CommandPool,uint32 ImagesCount,VkCommandBuffer * CommandBuffers)
{
    if (ImagesCount == 0) return 0;

    VkCommandBufferAllocateInfo CommandBufferAllocateInfo;

    CommandBufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; // VkStructureType   sType;
    CommandBufferAllocateInfo.pNext              = 0;                                              // Void * pNext;
    CommandBufferAllocateInfo.commandPool        = CommandPool;                    // VkCommandPool   commandPool;
    CommandBufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;                // VkCommandBufferLevel   level;
    CommandBufferAllocateInfo.commandBufferCount = ImagesCount;                                    // uint32_t   commandBufferCount;

    VK_CHECK(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo , CommandBuffers));

    return 0;
}

int32
WaitForRender()
{
    if (GlobalWindowIsMinimized) return 0;

    VK_CHECK(vkWaitForFences(GlobalVulkan.PrimaryDevice, 1, &GlobalVulkan.RenderFence, true, 1000000000));
    VK_CHECK(vkResetFences(GlobalVulkan.PrimaryDevice, 1, &GlobalVulkan.RenderFence));

    return 0;
}

int32
LoadShaderModule(char * Buffer, size_t Size)
{

    Assert((GlobalVulkan.ShaderModulesCount + 1) <= ArrayCount(GlobalVulkan.ShaderModules));

    VkShaderModule ShaderModule;

    VkShaderModuleCreateInfo ShaderModuleCreateInfo;
    ShaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO; // VkStructureType sType;
    ShaderModuleCreateInfo.pNext    = 0;                                           // Void * pNext;
    ShaderModuleCreateInfo.flags    = 0;                                           // VkShaderModuleCreateFlags flags;
    ShaderModuleCreateInfo.codeSize = Size;                                        // size_t codeSize;
    ShaderModuleCreateInfo.pCode    = (uint32 *)Buffer;                            // Typedef * pCode;

    VK_CHECK(vkCreateShaderModule(GlobalVulkan.PrimaryDevice, &ShaderModuleCreateInfo, 0, &ShaderModule));

    GlobalVulkan.ShaderModules[GlobalVulkan.ShaderModulesCount++] = ShaderModule;

    return 0;

}

int32
RenderLoop(real32 TimeElapsed)
{
    if (GlobalWindowIsMinimized) return 0;

    uint32 SwapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(GlobalVulkan.PrimaryDevice, GlobalVulkan.Swapchain, 1000000000, GlobalVulkan.ImageAvailableSemaphore, 0 , &SwapchainImageIndex));

    VK_CHECK(vkResetCommandBuffer(GlobalVulkan.PrimaryCommandBuffer[SwapchainImageIndex], 0));

    VkCommandBuffer cmd = GlobalVulkan.PrimaryCommandBuffer[0];

    VkCommandBufferBeginInfo CommandBufferBeginInfo;
    CommandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; // VkStructureType   sType;
    CommandBufferBeginInfo.pNext            = 0;                                           // Void * pNext;
    CommandBufferBeginInfo.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // VkCommandBufferUsageFlags   flags;
    CommandBufferBeginInfo.pInheritanceInfo = 0;                                           // Typedef * pInheritanceInfo;

    VK_CHECK(vkBeginCommandBuffer(cmd, &CommandBufferBeginInfo));

    real32 ColorRotation = sinf(TimeElapsed * (1.0f / 1.0f));
    VkClearValue ClearValue;
    ClearValue.color        = {0,0,ColorRotation,1.0f}; // VkClearColorValue   color;
    ClearValue.depthStencil = {};                       // VkClearDepthStencilValue   depthStencil;

    VkRenderPassBeginInfo RenderPassBeginInfo;
    RenderPassBeginInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;       // VkStructureType   sType;
    RenderPassBeginInfo.pNext           = 0;                                              // Void * pNext;
    RenderPassBeginInfo.renderPass      = GlobalVulkan.RenderPass;                        // VkRenderPass   renderPass;
    RenderPassBeginInfo.framebuffer     = GlobalVulkan.Framebuffers[SwapchainImageIndex]; // VkFramebuffer   framebuffer;
    RenderPassBeginInfo.renderArea      = {{},GlobalVulkan.WindowExtension};              // VkRect2D   renderArea;
    RenderPassBeginInfo.clearValueCount = 1;                                              // uint32_t   clearValueCount;
    RenderPassBeginInfo.pClearValues    = &ClearValue;                                    // Typedef * pClearValues;

    vkCmdBeginRenderPass(cmd, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdEndRenderPass(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkPipelineStageFlags WaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo SubmitInfo     = {};
    SubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;         // VkStructureType   sType;
    SubmitInfo.pNext                = 0;                                     // Void * pNext;
    SubmitInfo.waitSemaphoreCount   = 1;                                     // uint32_t   waitSemaphoreCount;
    SubmitInfo.pWaitSemaphores      = &GlobalVulkan.ImageAvailableSemaphore; // Typedef * pWaitSemaphores;
    SubmitInfo.pWaitDstStageMask    = &WaitStage;                            // Typedef * pWaitDstStageMask;
    SubmitInfo.commandBufferCount   = 1;                                     // uint32_t   commandBufferCount;
    SubmitInfo.pCommandBuffers      = &cmd;                                  // Typedef * pCommandBuffers;
    SubmitInfo.signalSemaphoreCount = 1;                                     // uint32_t   signalSemaphoreCount;
    SubmitInfo.pSignalSemaphores    = &GlobalVulkan.RenderSemaphore;         // Typedef * pSignalSemaphores;

    VK_CHECK(vkQueueSubmit(GlobalVulkan.GraphicsQueue, 1, &SubmitInfo, GlobalVulkan.RenderFence));

    VkPresentInfoKHR PresentInfo;
    PresentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR; // VkStructureType   sType;
    PresentInfo.pNext              = 0; // Void * pNext;
    PresentInfo.waitSemaphoreCount = 1; // uint32_t   waitSemaphoreCount;
    PresentInfo.pWaitSemaphores    = &GlobalVulkan.RenderSemaphore; // Typedef * pWaitSemaphores;
    PresentInfo.swapchainCount     = 1; // uint32_t   swapchainCount;
    PresentInfo.pSwapchains        = &GlobalVulkan.Swapchain; // Typedef * pSwapchains;
    PresentInfo.pImageIndices      = &SwapchainImageIndex; // Typedef * pImageIndices;
    PresentInfo.pResults           = 0; // Typedef * pResults;

    VK_CHECK(vkQueuePresentKHR(GlobalVulkan.GraphicsQueue, &PresentInfo));

    return 0;
}

void
FreeSwapchain()
{
    uint32 SwapchainImageCount = GlobalVulkan.SwapchainImageCount;

    if (SwapchainImageCount > 0)
    {
        for (uint32 ImageIndex = 0;
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

        vkFreeCommandBuffers(GlobalVulkan.PrimaryDevice,GlobalVulkan.CommandPool, SwapchainImageCount,GlobalVulkan.PrimaryCommandBuffer);

        if ( VK_VALID_HANDLE(GlobalVulkan.RenderPass) )
        {
            vkDestroyRenderPass(GlobalVulkan.PrimaryDevice, GlobalVulkan.RenderPass, 0);
        }


        GlobalVulkan.SwapchainImageCount = 0;
    }

}
int32
VulkanRenewSwapChain(int32 Width,int32 Height)
{
    if (GlobalVulkan.Initialized)
    {
        if (GlobalWindowIsMinimized && Width > 0)
        {
            GlobalWindowIsMinimized = true;
        }

        VulkanWaitForDevices();

        FreeSwapchain();
        
        if (VulkanCreateSwapChain(Width, Height)) return 1;

        // If window is minimized process is halt
        if (GlobalWindowIsMinimized) return 0;

        if (VulkanCreateCommandBuffers(
                    GlobalVulkan.PrimaryDevice,
                    GlobalVulkan.CommandPool,
                    GlobalVulkan.SwapchainImageCount,
                    GlobalVulkan.PrimaryCommandBuffer)) return 1;

        if (VulkanInitDefaultRenderPass()) return 1;

        if (VulkanInitFramebuffers()) return 1;
    }

    return 0;
}

int32
VulkanCreateLogicaDevice()
{
    vulkan_device_extensions VulkanDeviceExt = GetRequiredDeviceExtensions();
    const char ** DeviceExtensionsRequired = &VulkanDeviceExt.DeviceExtensions[0];
    uint32 TotalDeviceExtReq = VulkanDeviceExt.Count;

    real32 QueuePriorities[1] = { 1.0f };
    VkDeviceQueueCreateInfo QueueCreateInfo[2];
    uint32 QueuesRequired = 1;

    QueueCreateInfo[0] = {
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     // VkStructureType              sType
      nullptr,                                        // const void                  *pNext
      0,                                              // VkDeviceQueueCreateFlags     flags
      GlobalVulkan.GraphicsQueueFamilyIndex,          // uint32_t                     queueFamilyIndex
      ArrayCount(QueuePriorities),                    // uint32_t                     queueCount
      &QueuePriorities[0]                             // const float                 *pQueuePriorities
    };

    if (GlobalVulkan.GraphicsQueueFamilyIndex != GlobalVulkan.PresentationQueueFamilyIndex)
    {
        ++QueuesRequired;
        QueueCreateInfo[0] = {
          VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,         // VkStructureType              sType
          nullptr,                                            // const void                  *pNext
          0,                                                  // VkDeviceQueueCreateFlags     flags
          GlobalVulkan.PresentationQueueFamilyIndex,          // uint32_t                     queueFamilyIndex
          ArrayCount(QueuePriorities),                        // uint32_t                     queueCount
          &QueuePriorities[0]                                 // const float                 *pQueuePriorities
        };
    }

    VkDeviceCreateInfo DeviceCreateInfo = {};
    DeviceCreateInfo.sType                       = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; // VkStructureType   sType;
    DeviceCreateInfo.pNext                       = 0;                                    // Void * pNext;
    DeviceCreateInfo.flags                       = 0;                                    // VkDeviceCreateFlags   flags;
    DeviceCreateInfo.queueCreateInfoCount        = QueuesRequired;                       // uint32_t   queueCreateInfoCount;
    DeviceCreateInfo.pQueueCreateInfos           = &QueueCreateInfo[0];                  // Typedef * pQueueCreateInfos;
    // DEPRECATED
    DeviceCreateInfo.enabledLayerCount           = 0;                                    // uint32_t   enabledLayerCount;
    // DEPRECATED
    DeviceCreateInfo.ppEnabledLayerNames         = 0;                                    // Pointer * ppEnabledLayerNames;
    DeviceCreateInfo.enabledExtensionCount       = TotalDeviceExtReq; // uint32_t   enabledExtensionCount;
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

    vkGetDeviceQueue(GlobalVulkan.PrimaryDevice, GlobalVulkan.GraphicsQueueFamilyIndex, 0, &GlobalVulkan.GraphicsQueue);
    vkGetDeviceQueue(GlobalVulkan.PrimaryDevice, GlobalVulkan.PresentationQueueFamilyIndex, 0, &GlobalVulkan.PresentationQueue);

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateSwapchainKHR);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroySwapchainKHR);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkGetSwapchainImagesKHR);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkAcquireNextImageKHR);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkQueuePresentKHR);

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateRenderPass);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyRenderPass);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateImageView);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyImageView);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateFramebuffer);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyFramebuffer);

    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkCreateShaderModule);
    VK_DEVICE_LEVEL_FN(GlobalVulkan.PrimaryDevice,vkDestroyShaderModule);

    VkFenceCreateInfo FenceCreateInfo;
    FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO; // VkStructureType   sType;
    FenceCreateInfo.pNext = 0;                                   // Void * pNext;
    FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;        // VkFenceCreateFlags   flags;

    VK_CHECK(vkCreateFence(GlobalVulkan.PrimaryDevice, &FenceCreateInfo, 0,&GlobalVulkan.RenderFence));

    VkSemaphoreCreateInfo SemaphoreCreateInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        0,
        0
    };

    VK_CHECK(vkCreateSemaphore(GlobalVulkan.PrimaryDevice, &SemaphoreCreateInfo, 0, &GlobalVulkan.ImageAvailableSemaphore));
    VK_CHECK(vkCreateSemaphore(GlobalVulkan.PrimaryDevice, &SemaphoreCreateInfo, 0, &GlobalVulkan.RenderSemaphore));

    return 0;
}

vk_version
GetVkVersionFromUint32(uint32 Version)
{
    vk_version Result = {
        VK_VERSION_MAJOR(Version),
        VK_VERSION_MINOR(Version),
        VK_VERSION_PATCH(Version)
    };
    return Result;
}


int32
VulkanGetPhysicalDevice()
{
    vulkan_device_extensions VulkanDeviceExt = GetRequiredDeviceExtensions();
    const char ** DeviceExtensionsRequired = &VulkanDeviceExt.DeviceExtensions[0];
    uint32 TotalDeviceExtReq = VulkanDeviceExt.Count;

    uint32 TotalPhysicalDevices = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(GlobalVulkan.Instance,&TotalPhysicalDevices,0));

    VkPhysicalDevice PhysicalDevices[12];
    Assert(ArrayCount(PhysicalDevices) >= TotalPhysicalDevices);
    VK_CHECK(vkEnumeratePhysicalDevices(GlobalVulkan.Instance,&TotalPhysicalDevices,&PhysicalDevices[0]));

    VkPhysicalDevice SelectedDeviceHandle = VK_NULL_HANDLE;
    vk_version SelectedVersion = {};
    uint32 SelectedMaxImageDimension2D = 0;
    uint32 SelectedQueueFamilyIndex = UINT32_MAX;

    for (uint32 i = 0;
         i < TotalPhysicalDevices;
         ++i)
    {
        VkPhysicalDeviceProperties  PhysicalDeviceProperties;
        vkGetPhysicalDeviceProperties(PhysicalDevices[i], &PhysicalDeviceProperties);

        VkPhysicalDeviceFeatures    PhysicalDeviceFeatures;
        vkGetPhysicalDeviceFeatures(PhysicalDevices[i],   &PhysicalDeviceFeatures);

        vk_version Version = GetVkVersionFromUint32(PhysicalDeviceProperties.apiVersion);

        uint32 TotalQueueFamilyPropertyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevices[i],&TotalQueueFamilyPropertyCount,0);

        if (TotalQueueFamilyPropertyCount > 0)
        {

            // TODO: Family queues can have bigger internal queues count and 
            //       more flags which might be preferred.
            //       Need to revisit and understand if makes impact
            uint32 GraphicsQueueFamilyIndex     = UINT32_MAX;
            uint32 PresentationQueueFamilyIndex = UINT32_MAX;

            VkQueueFamilyProperties QueueFamiliyProperties[10];
            Assert(ArrayCount(QueueFamiliyProperties) >= TotalQueueFamilyPropertyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevices[i],&TotalQueueFamilyPropertyCount,&QueueFamiliyProperties[0]);

            for (uint32 QueueFamilyIndex = 0;
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

            bool32 QueueFamilySuffice = (
                    (PresentationQueueFamilyIndex != UINT32_MAX) && 
                    (GraphicsQueueFamilyIndex     != UINT32_MAX)
            );

            uint32 TotalDeviceExtProp = 0;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(PhysicalDevices[i], 0, &TotalDeviceExtProp, 0));

            VkExtensionProperties DeviceExtensionProps[120] = {};
            Assert(ArrayCount(DeviceExtensionProps) >= TotalDeviceExtProp);
            VK_CHECK(vkEnumerateDeviceExtensionProperties(PhysicalDevices[i], 0, &TotalDeviceExtProp, &DeviceExtensionProps[0]));

            bool32 AllDeviceExtSupported = true;
            for (uint32 RequiredDeviceExtPropIndex = 0;
                 RequiredDeviceExtPropIndex < TotalDeviceExtReq;
                 ++RequiredDeviceExtPropIndex)
            {
                uint32 DeviceExtPropIndex = 0;
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
                // this does not works. driver has x.x.x.x
                vk_version DriverVersion = GetVkVersionFromUint32(PhysicalDeviceProperties.driverVersion);
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

bool32
VulkanInstanceSupportsSurface(const char * VulkanSurfaceExtensionNameOS)
{
    uint32 TotalInstanceExtensionProp = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &TotalInstanceExtensionProp, 0));

    VkExtensionProperties InstanceExtensionProp[20] = {};
    Assert(ArrayCount(InstanceExtensionProp) >= TotalInstanceExtensionProp);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &TotalInstanceExtensionProp, &InstanceExtensionProp[0]));

    bool32 SurfaceSupport       = false;
    bool32 SurfaceSupportOS     = false;

    for (uint32 i = 0;
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


int32
VulkanGetInstance(bool32 EnableValidationLayer, 
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
    AppInfo.applicationVersion    = RENDER_VERSION; // uint32_t   applicationVersion;
    AppInfo.pEngineName           = ""; // Char_S * pEngineName;
    AppInfo.engineVersion         = ENGINE_VERSION; // uint32_t   engineVersion;
    AppInfo.apiVersion            = VULKAN_API_VERSION; // uint32_t   apiVersion;
    
    if (!VulkanInstanceSupportsSurface(VkKHROSSurfaceExtensionName))
    {
        Log("Instance does not supports surfaces");
        return 1;
    }

    const char * ExtensionsRequired[2] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VkKHROSSurfaceExtensionName
    };

    int32 EnabledInstanceLayerCount = 0;
    const char * AllInstanceLayers[1] = {
        "VK_LAYER_KHRONOS_validation"
    };
    bool32 InstanceLayersEnabledStatus[ArrayCount(AllInstanceLayers)] = {
        EnableValidationLayer 
    };

    char * InstanceLayersToEnable[ArrayCount(AllInstanceLayers)];

    uint32 InstanceLayerCount;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&InstanceLayerCount,0));

    VkLayerProperties LayerProperties[40];
    Assert(ArrayCount(LayerProperties) >= InstanceLayerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&InstanceLayerCount,&LayerProperties[0]));

    for (uint32 LayerIndex = 0;
                LayerIndex < ArrayCount(InstanceLayersEnabledStatus);
                ++LayerIndex)
    {
        if ( InstanceLayersEnabledStatus[LayerIndex] )
        {
            bool32 Found  = false;
            for (uint32 LayerPropIndex = 0;
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
    InstanceCreateInfo.enabledLayerCount       = EnabledInstanceLayerCount;              // uint32_t   enabledLayerCount;
    if (EnabledInstanceLayerCount)
    {
        InstanceCreateInfo.ppEnabledLayerNames = &InstanceLayersToEnable[0];             // Pointer * ppEnabledLayerNames;
    }
    InstanceCreateInfo.enabledExtensionCount   = ArrayCount(ExtensionsRequired);         // uint32_t   enabledExtensionCount;
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
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdBeginRenderPass);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdEndRenderPass);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdPipelineBarrier);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdClearColorImage);
    VK_INSTANCE_LEVEL_FN(Instance,vkQueueSubmit);

    return 0;
}

int32
InitializeVulkan(int32 Width, int32 Height, 
                 vulkan_platform_window PlatformWindow,
                 bool32 EnableValidationLayer,
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

    if (VulkanCreateCommandPool()) return 1;

    if (VulkanCreateCommandBuffers(
                GlobalVulkan.PrimaryDevice,
                GlobalVulkan.CommandPool,
                GlobalVulkan.SwapchainImageCount,
                GlobalVulkan.PrimaryCommandBuffer)) return 1;

    if (VulkanInitDefaultRenderPass()) return 1;

    if (VulkanInitFramebuffers()) return 1;

    GlobalVulkan.Initialized = true;

    return 0;
}


void
CloseVulkan()
{

    VulkanWaitForDevices();

    /* ALWAYS AFTER WAIT IDLE */

    FreeSwapchain();

    if (VK_VALID_HANDLE(GlobalVulkan.CommandPool)) 
    {
        vkDestroyCommandPool(GlobalVulkan.PrimaryDevice,GlobalVulkan.CommandPool,0);
    }

    if (VK_VALID_HANDLE(GlobalVulkan.ImageAvailableSemaphore))
    {
        vkDestroySemaphore(GlobalVulkan.PrimaryDevice, GlobalVulkan.ImageAvailableSemaphore, 0);
    }

    if (VK_VALID_HANDLE(GlobalVulkan.RenderSemaphore))
    {
        vkDestroySemaphore(GlobalVulkan.PrimaryDevice, GlobalVulkan.RenderSemaphore, 0);
    }

    if (VK_VALID_HANDLE(GlobalVulkan.RenderFence))
    {
        vkDestroyFence(GlobalVulkan.PrimaryDevice, GlobalVulkan.RenderFence, 0);
    }

    if ( VK_VALID_HANDLE(GlobalVulkan.Swapchain) )
    {
        vkDestroySwapchainKHR(GlobalVulkan.PrimaryDevice, GlobalVulkan.Swapchain, 0);
    }

    for (uint32 ShaderIndex = 0;
                ShaderIndex < GlobalVulkan.ShaderModulesCount;
                ++ShaderIndex)
    {
        Assert(VK_VALID_HANDLE(GlobalVulkan.ShaderModules[ShaderIndex]));
        vkDestroyShaderModule(GlobalVulkan.PrimaryDevice,GlobalVulkan.ShaderModules[ShaderIndex],0);
        GlobalVulkan.ShaderModules[ShaderIndex] = VK_NULL_HANDLE;
    }

    if (VK_VALID_HANDLE(GlobalVulkan.PrimaryDevice) )vkDestroyDevice(GlobalVulkan.PrimaryDevice,0);

    if (VK_VALID_HANDLE(GlobalVulkan.SecondaryDevice) )vkDestroyDevice(GlobalVulkan.SecondaryDevice,0);

    if (VK_VALID_HANDLE(GlobalVulkan.Surface) )vkDestroySurfaceKHR(GlobalVulkan.Instance, GlobalVulkan.Surface, 0);

    if (VK_VALID_HANDLE(GlobalVulkan.Instance) )vkDestroyInstance(GlobalVulkan.Instance,0);
}
