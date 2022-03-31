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
#include "graphics_api.h"
#include "vulkan_local.h"
#include "game_memory.h"

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
PFN_vkGetDeviceImageMemoryRequirements        vkGetDeviceImageMemoryRequirements        = 0;
PFN_vkGetImageMemoryRequirements              vkGetImageMemoryRequirements              = 0;
PFN_vkMapMemory                               vkMapMemory                               = 0;
PFN_vkUnmapMemory                             vkUnmapMemory                             = 0;
PFN_vkBindBufferMemory                        vkBindBufferMemory                        = 0;
PFN_vkBindImageMemory                         vkBindImageMemory                         = 0;

// GLOBALS
vulkan                GlobalVulkan                = {};
b32                   GlobalWindowIsMinimized     = false;
vulkan_destroy_queue  PrimaryVulkanDestroyQueue   = {};


#define QUEUE_DELETE(queue,func_name, CountParams,Param1,Param2,Param3, Param4, Param5) queue_delete_(queue,vulkan_destructor_type_##func_name,CountParams, (void *)Param1, (void *)Param2,(void *)Param3, (void *)Param4,(void *)Param5)

#define QUEUE_DELETE_SURFACEKHR(queue,instance,surface)                    QUEUE_DELETE(queue,vkDestroySurfaceKHR,2,instance,surface,0,0,0)
#define QUEUE_DELETE_SWAPCHAINKHR(queue,device,swapchain)                  QUEUE_DELETE(queue,vkDestroySwapchainKHR,2,device,swapchain,0,0,0)
#define QUEUE_DELETE_INSTANCE(queue,instance)                              QUEUE_DELETE(queue,vkDestroyInstance,1,instance,0,0,0,0)
#define QUEUE_DELETE_COMMANDPOOL(queue,device,commandPool)                 QUEUE_DELETE(queue,vkDestroyCommandPool,2,device,commandPool,0,0,0)
#define QUEUE_DELETE_SHADERMODULE(queue,device,shaderModule)               QUEUE_DELETE(queue,vkDestroyShaderModule,2,device,shaderModule,0,0,0)
#define QUEUE_DELETE_DEVICE(queue,device)                                  QUEUE_DELETE(queue,vkDestroyDevice,1,device,0,0,0,0)
#define QUEUE_DELETE_FENCE(queue,device,fence)                             QUEUE_DELETE(queue,vkDestroyFence,2,device,fence,0,0,0)
#define QUEUE_DELETE_SEMAPHORE(queue,device,semaphore)                     QUEUE_DELETE(queue,vkDestroySemaphore,2,device,semaphore,0,0,0)
#define QUEUE_DELETE_RENDERPASS(queue,device,renderPass)                   QUEUE_DELETE(queue,vkDestroyRenderPass,2,device,renderPass,0,0,0)
#define QUEUE_DELETE_SAMPLER(queue,device,sampler)                         QUEUE_DELETE(queue,vkDestroySampler,2,device,sampler,0,0,0)
#define QUEUE_DELETE_IMAGEVIEW(queue,device,imageView)                     QUEUE_DELETE(queue,vkDestroyImageView,2,device,imageView,0,0,0)
#define QUEUE_DELETE_FRAMEBUFFER(queue,device,framebuffer)                 QUEUE_DELETE(queue,vkDestroyFramebuffer,2,device,framebuffer,0,0,0)
#define QUEUE_DELETE_PIPELINE(queue,device,pipeline)                       QUEUE_DELETE(queue,vkDestroyPipeline,2,device,pipeline,0,0,0)
#define QUEUE_DELETE_PIPELINELAYOUT(queue,device,pipelineLayout)           QUEUE_DELETE(queue,vkDestroyPipelineLayout,2,device,pipelineLayout,0,0,0)
#define QUEUE_DELETE_DESCRIPTORSETLAYOUT(queue,device,descriptorSetLayout) QUEUE_DELETE(queue,vkDestroyDescriptorSetLayout,2,device,descriptorSetLayout,0,0,0)
#define QUEUE_DELETE_DESCRIPTORPOOL(queue,device,descriptorPool)           QUEUE_DELETE(queue,vkDestroyDescriptorPool,2,device,descriptorPool,0,0,0)
#define QUEUE_DELETE_BUFFER(queue,device,buffer)                           QUEUE_DELETE(queue,vkDestroyBuffer,2,device,buffer,0,0,0)
#define QUEUE_DELETE_ARENA(queue,arena)                                    QUEUE_DELETE(queue,vkDestroyArenaCustom,1,arena,0,0,0,0)
#define QUEUE_DELETE_IMAGE(queue,device,image)                             QUEUE_DELETE(queue,vkDestroyImage,2,device,image,0,0,0)
#define QUEUE_DELETE_MEMORY_POOL(queue,memory_pool)                        QUEUE_DELETE(queue,vkDestroyMemoryPoolCustom,1,memory_pool,0,0,0,0)
//#define QUEUE_DELETE_IMAGE_CUSTOM(queue,image)                             QUEUE_DELETE(queue,vkDestroyImageCustom,1,image,0,0,0,0)

void
queue_delete_(vulkan_destroy_queue * Queue, vulkan_destructor_type DestructorType, u32 CountParams, void * Param1, void * Param2, void * Param3,void * Param4, void * Param5)
{
    Assert(Queue->ItemsCount < ArrayCount(Queue->Items));

    vulkan_destroy_queue_item * Item = Queue->Items + Queue->ItemsCount;

    Item->DestructorType = DestructorType;

    if (CountParams >= 1) Item->Params[0] = Param1;
    if (CountParams >= 2) Item->Params[1] = Param2;
    if (CountParams >= 3) Item->Params[2] = Param3;
    if (CountParams >= 4) Item->Params[3] = Param3;

    Assert(CountParams > 0 && CountParams <= 4);

    Item->CountParams = CountParams;
    Queue->ItemsCount += 1;
}


i32
VH_CreateImage(VkPhysicalDevice PhysicalDevice,VkDevice Device, u32 Width, u32 Height, u32 Channels, vulkan_image * Image)
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
    Assert(MemoryTypeIndex == GlobalVulkan.TextureArena->MemoryIndexType);

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

            VulkanPipeline->Viewport       = VH_CreateDefaultViewport(GlobalVulkan.WindowExtension); // VkViewport Viewport;

            VulkanPipeline->Scissor.offset = {0,0};
            VulkanPipeline->Scissor.extent = GlobalVulkan.WindowExtension;

            VkPipeline Pipeline = VH_PipelineBuilder(VulkanPipeline, GlobalVulkan.PrimaryDevice, GlobalVulkan.RenderPass);

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


GRAPHICS_DESTROY_MATERIAL_PIPELINE(FreeMaterialPipeline)
{
    Assert(Index >= 0 && Index < ArrayCount(GlobalVulkan.Pipelines));
    VkPipeline * Pipeline = GlobalVulkan.Pipelines + Index;
    if (VK_VALID_HANDLE(*Pipeline))
    {
        vkDestroyPipeline(GlobalVulkan.PrimaryDevice,*Pipeline,0);
        *Pipeline = VK_NULL_HANDLE;
    }
    else
    {
        Logn("Pipeline was not created");
    }
}

GRAPHICS_CREATE_MATERIAL_PIPELINE(CreatePipeline)
{

    pipeline_creation_result Result = {};

    Assert((VertexShaderIndex >= 0) && (ArrayCount(GlobalVulkan.ShaderModules) > VertexShaderIndex));
    Assert((FragmentShaderIndex >= 0) && (ArrayCount(GlobalVulkan.ShaderModules) > FragmentShaderIndex));

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

    VulkanPipeline.Rasterizer           = 
        VH_CreatePipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL); // VkPipelineRasterizationStateCreateInfo   Rasterizer;

    VulkanPipeline.ColorBlendAttachment = VH_CreatePipelineColorBlendAttachmentState();  // VkPipelineColorBlendAttachmentState ColorBlendAttachment;
    VulkanPipeline.Multisampling        = VH_CreatePipelineMultisampleStateCreateInfo(); // VkPipelineMultisampleStateCreateInfo Multisampling;
    VulkanPipeline.PipelineLayout       = GlobalVulkan.PipelineLayout[0];                      // VkPipelineLayout PipelineLayout;

    VkPipeline Pipeline = VH_PipelineBuilder(&VulkanPipeline, GlobalVulkan.PrimaryDevice, GlobalVulkan.RenderPass);

    if (VK_VALID_HANDLE(Pipeline))
    {
        Assert(GlobalVulkan.PipelinesCount < ArrayCount(GlobalVulkan.Pipelines));

        i32 IndexPipeline = GlobalVulkan.PipelinesCount++;

        GlobalVulkan.PipelinesDefinition[IndexPipeline] = VulkanPipeline;
        GlobalVulkan.Pipelines[IndexPipeline]= Pipeline;

        Result.Pipeline = IndexPipeline;
        Result.Success = true;
        Result.PipelineLayout = 0; // only 1 for now, hardcoded as the first 

        QUEUE_DELETE_PIPELINE(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &GlobalVulkan.Pipelines[IndexPipeline]);
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

gpu_memory_mapping_result
BeginObjectMapping(u32 Units)
{
    gpu_memory_mapping_result Result = {};

    gpu_arena * Arena = GetCurrentFrame()->ObjectsArena;
    u32 BeginOffsetUnits = Arena->CurrentSize / sizeof(GPUObjectData);
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
    DepthAttachment.format         = GlobalVulkan.PrimaryDepthBuffer->Format;                  // VkFormat format;
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

    vkCmdPushConstants(cmd,GlobalVulkan.CurrentPipelineLayout,VK_SHADER_STAGE_VERTEX_BIT,0,Size,Data);
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
VH_WriteDescriptorImage(u32 BindingSlot,VkDescriptorSet Set,VkDescriptorType DescriptorType, VkDescriptorImageInfo * ImageInfo)
{
    VkWriteDescriptorSet WriteDescriptor;

    WriteDescriptor.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; // VkStructureType   sType;
    WriteDescriptor.pNext            = 0; // Void * pNext;
    WriteDescriptor.dstSet           = Set; // VkDescriptorSet   dstSet;
    WriteDescriptor.dstBinding       = BindingSlot; // uint32_t   dstBinding;
    WriteDescriptor.dstArrayElement  = 0; // uint32_t   dstArrayElement;
    WriteDescriptor.descriptorCount  = 1; // uint32_t   descriptorCount;
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

GRAPHICS_PUSH_TEXTURE_DATA(PushTextureData)
{

    i32 ResultImageIndex = -1;

    VkDeviceSize Offset = 0;

    gpu_arena * Arena = GlobalVulkan.TextureArena;
    Assert(Arena->ImageCount < ArrayCount(Arena->Images));
    vulkan_image * VulkanImage = Arena->Images + Arena->ImageCount;

    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkMemoryMapFlags.html
    VkMemoryMapFlags Flags = 0; // RESERVED FUTURE USE

    if (VH_CreateImage(GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice, Width,Height,Channels, VulkanImage)) 
        return ResultImageIndex;

    VkDeviceMemory DeviceMemory = GetDeviceMemory(Arena->MemoryIndexType);
    VkDeviceSize BindMemoryOffset = Arena->DeviceBindingOffsetBegin + Arena->CurrentSize;

    if (VK_FAILS(vkBindImageMemory(GlobalVulkan.PrimaryDevice, VulkanImage->Image,DeviceMemory, BindMemoryOffset)))
    {
        VH_DestroyImage(Arena->Device,VulkanImage);
        return ResultImageIndex;
    }
            
    VkDeviceSize DeviceSize = Width * Height * Channels;

    /*
     * 1) Copy CPU to GPU visible memory
     * 2) Transition Image as destination
     * 3) Copy GPU temp memory to Image
     * 4) Transition Image to readable by shaders
     */
    VulkanWriteDataToArena(GlobalVulkan.TransferBitArena, Data, (u32)DeviceSize);

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

    // TODO: figure out how to manage descriptor set 
    // for multiple images
    // I think having big textures with all data
    // and setting them once at the beginning of the frame
    // but you will need to cover all texture requirements
    // for the frame
    if (!VK_VALID_HANDLE(GlobalVulkan._DebugTextureSet))
    {
        VH_AllocateDescriptor(&GlobalVulkan._DebugTextureSetLayout,GlobalVulkan._DescriptorPool,&GlobalVulkan._DebugTextureSet);
    }

    Arena->CurrentSize += (u32)VulkanImage->MemoryRequirements.size;

    // commit
    ResultImageIndex = Arena->ImageCount++;

    return ResultImageIndex;
}

i32
RenderBindTexture(u32 ImageIndex)
{
    u32 TextureBindingSlot = 2;
    
    VkPipelineLayout PipelineLayout = 
        GlobalVulkan.CurrentPipelineLayout;

    Assert(VK_VALID_HANDLE(PipelineLayout));

    vulkan_image * Image = GlobalVulkan.TextureArena->Images + ImageIndex;

    VkDescriptorImageInfo ImageInfo;
    ImageInfo.sampler     = GlobalVulkan.TextureSampler; // VkSampler   sampler;
    ImageInfo.imageView   = Image->ImageView; // VkImageView   imageView;
    ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // VkImageLayout   imageLayout;

    VkWriteDescriptorSet ImageWriteSet = 
        VH_WriteDescriptorImage(0, 
                                GlobalVulkan._DebugTextureSet, 
                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
                                &ImageInfo);

    vkUpdateDescriptorSets(GlobalVulkan.PrimaryDevice,1, &ImageWriteSet,  0, nullptr);

    vkCmdBindDescriptorSets(GetCurrentFrame()->PrimaryCommandBuffer, 
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            PipelineLayout, 
                            TextureBindingSlot, 
                            1, 
                            &GlobalVulkan._DebugTextureSet, 
                            0, nullptr);

    return 0;
}


GRAPHICS_PUSH_VERTEX_DATA(PushVertexData)
{

    gpu_arena * VertexArena = GlobalVulkan.VertexArena;
    VkDeviceSize Offset = VertexArena->CurrentSize;

    u32 Align = VertexArena->Alignment - 1;
    u32 DataSizeAligned = (DataSize + Align) & ~Align;

    Assert((VertexArena->CurrentSize + DataSizeAligned) < VertexArena->MaxSize);

    VulkanWriteDataToArena(GlobalVulkan.TransferBitArena,Data, DataSize);

    if (VH_CopyBuffer(GlobalVulkan.TransferBitCommandBuffer, 
                     GlobalVulkan.TransferBitArena->Buffer,VertexArena->Buffer, DataSize, Offset))
    {
        Log("Failed to copy data from buffer to gpu\n");
        return 1;
    }

    VertexArena->CurrentSize += DataSizeAligned;

    *outVertexBufferBeginOffset = (u32)Offset;

    return 0;
}


i32
RenderPushIndexData(void * Data,u32 DataSize, u32 BaseOffset)
{
    gpu_arena * IndexArena = GlobalVulkan.IndexArena;
    VkDeviceSize Offset = IndexArena->CurrentSize;

    u32 Align = IndexArena->Alignment - 1;
    DataSize = (DataSize + Align) & ~Align;

    Assert((IndexArena->CurrentSize + DataSize) < IndexArena->MaxSize);

    IndexArena->CurrentSize += DataSize;

    VkDeviceSize DeviceSize = DataSize;

    VulkanWriteDataToArena(GlobalVulkan.TransferBitArena,Data, DataSize);

    if (VH_CopyBuffer(GlobalVulkan.TransferBitCommandBuffer, 
                     GlobalVulkan.TransferBitArena->Buffer,IndexArena->Buffer, DataSize, Offset))
    {
        Log("Failed to copy data from buffer to gpu\n");
        return 1;
    }

    //*outIndexBufferBeginOffset = (u32)Offset;

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

GRAPHICS_DELETE_SHADER_MODULE(DeleteShaderModule)
{

    Assert(ShaderIndex >= 0 && ShaderIndex < ArrayCount(GlobalVulkan.ShaderModules));
    Assert(VK_VALID_HANDLE(GlobalVulkan.ShaderModules[ShaderIndex]));

    vkDestroyShaderModule(GlobalVulkan.PrimaryDevice,GlobalVulkan.ShaderModules[ShaderIndex],0);
    GlobalVulkan.ShaderModules[ShaderIndex] = VK_NULL_HANDLE;
}

GRAPHICS_CREATE_SHADER_MODULE(CreateShaderModule)
{

    i32 ShaderIndex;
    VkShaderModule * ShaderModule = 0;

    for (ShaderIndex = 0; ShaderIndex < ArrayCount(GlobalVulkan.ShaderModules); ++ShaderIndex)
    {
        ShaderModule = GlobalVulkan.ShaderModules + ShaderIndex;
        if ((*ShaderModule) == VK_NULL_HANDLE)
        {
            break;
        }
    }

    // not enough cache? sync with cpu assest load model
    Assert(ShaderIndex < ArrayCount(GlobalVulkan.ShaderModules));

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

    QUEUE_DELETE_SHADERMODULE(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,ShaderModule); 

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

    u32 CurrentFrameIndex = GlobalVulkan._CurrentFrameData % FRAME_OVERLAP;
    u32 UniformOffset = (u32)VH_PaddedUniformBuffer(sizeof(GPUSimulationData)) * CurrentFrameIndex;

    VkPipelineLayout PipelineLayout = GlobalVulkan.PipelineLayout[0];

    // Global descriptor, bind once
    vkCmdBindDescriptorSets(cmd, 
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            PipelineLayout, 
                            0, 1, 
                            &GetCurrentFrame()->GlobalDescriptor, 
                            1, 
                            &UniformOffset);

    vkCmdBindDescriptorSets(cmd, 
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            PipelineLayout, 
                            1, 1, 
                            &GetCurrentFrame()->ObjectsDescriptor, 
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

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);

    u32 CurrentFrameIndex = GlobalVulkan._CurrentFrameData % FRAME_OVERLAP;

    VkPipelineLayout PipelineLayout = 
        GlobalVulkan.PipelinesDefinition[PipelineIndex].PipelineLayout;

    GlobalVulkan.CurrentPipelineLayout = PipelineLayout;

    return 0;
}

#if 0
i32
RenderPushMeshIndexed(u32 TotalMeshInstances, u32 IndicesSize, VkDeviceSize OffsetVertex, VkDeviceSize OffsetIndices)
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;

    vkCmdBindVertexBuffers(cmd, 0, 1, &GlobalVulkan.VertexBuffer.Buffer, &OffsetVertex);
    vkCmdBindIndexBuffer(cmd, GlobalVulkan.IndexBuffer.Buffer, OffsetIndices, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd,IndicesSize,1,0,0,0);

    return 0;
}
#endif
i32
RenderDrawObject(u32 VertexSize,u32 FirstInstance)
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;

    vkCmdDraw(cmd,VertexSize, 1, 0 , FirstInstance);

    return 0;
}
i32
RenderDrawObjectNTimes(u32 VertexSize, u32 NTimes, u32 FirstInstance)
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;

    vkCmdDraw(cmd,VertexSize, NTimes, 0 , FirstInstance);

    return 0;
}

i32
RenderBindMesh(u32 VertexSize, u32 Offset)
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;

    VkDeviceSize OffsetVertex = Offset;
    vkCmdBindVertexBuffers(cmd, 0, 1, &GlobalVulkan.VertexArena->Buffer, &OffsetVertex);

    return 0;
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
RenderEndPassNoPresent()
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;
    u32 SwapchainImageIndex = GlobalVulkan.CurrentSwapchainImageIndex;

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


GRAPHICS_ON_WINDOW_RESIZE(OnWindowResize)
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

            if (VH_CreateCommandBuffers(
                        GlobalVulkan.PrimaryDevice,
                        FrameData->CommandPool,
                        1,
                        &FrameData->PrimaryCommandBuffer)) return 1;
        }

        if (VulkanInitDefaultRenderPass()) return 1;

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
    PhysicalDeviceFeatures2.features.geometryShader = VK_TRUE;
    PhysicalDeviceFeatures2.features.samplerAnisotropy = VK_TRUE;

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

        b32 AllFeaturesSupported = SupportsAnisotropyFilter &&
                                   SupportsGeometryShaders  &&
                                   SupportsBindingDescriptorDuringCommand
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
    VkDescriptorSetLayoutBinding LayoutBinding = 
        VH_CreateDescriptorSetLayoutBinding(0,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding DescriptorSetLayoutBindings[] = 
    {
        LayoutBinding
    };


    // TODO: how can we avoid allowing rebinding
    // thsi will be solved asap I figure out how to deal
    // with image descriptor at the beginning of the frame
    // before initializing command buffer
    VkDescriptorBindingFlags BindingFlags = VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo  BindingFlagsCreateInfo;
    BindingFlagsCreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO; // sType   sType
    BindingFlagsCreateInfo.pNext         = 0; // Void * pNext
    BindingFlagsCreateInfo.bindingCount  = 1; // bindingCount   bindingCount
    BindingFlagsCreateInfo.pBindingFlags = &BindingFlags; // VkDescriptorBindingFlags * pBindingFlags

    VkDescriptorSetLayoutCreateInfo SetInfo;
    SetInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO; // VkStructureType   sType;
    SetInfo.pNext        = &BindingFlagsCreateInfo; // Void * pNext;
    SetInfo.flags        = 0; // VkDescriptorSetLayoutCreateFlags   flags;
    SetInfo.bindingCount = ArrayCount(DescriptorSetLayoutBindings); // uint32_t   bindingCount;
    SetInfo.pBindings    = &DescriptorSetLayoutBindings[0]; // Typedef * pBindings;

    VK_CHECK(vkCreateDescriptorSetLayout(GlobalVulkan.PrimaryDevice,&SetInfo,nullptr,&GlobalVulkan._DebugTextureSetLayout));

    return 0;
}





i32
CreatePipelineLayoutTexture(VkPipelineLayout * PipelineLayout)
{
    VkDescriptorSetLayout LayoutSets[] = 
    {
        GlobalVulkan._GlobalSetLayout,
        GlobalVulkan._ObjectsSetLayout,
        GlobalVulkan._DebugTextureSetLayout
    };

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = VH_CreatePipelineLayoutCreateInfo();

    VkPushConstantRange PushConstant;
    PushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // VkShaderStageFlags   stageFlags;
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
CreateUnallocatedMemoryArenas(gpu_arena * Arenas, i32 MaxArenas, u32 * ArenasCreatedCount)
{
    i32 ArenaCount = 0;

    // Single uniform buffer shared by each FRAME_OVERLAP
    VkDeviceSize SimulationBufferPaddedSize = FRAME_OVERLAP * VH_PaddedUniformBuffer(sizeof(GPUSimulationData));
    Assert(ArenaCount < MaxArenas);
    gpu_arena * SimulationArena = Arenas + ArenaCount++;
    if (VH_CreateUnAllocArenaBuffer(
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

    // Maximum number of entities rendered
    u32 TotalFrameObjects = 10000;
    u32 ObjectsSize = sizeof(GPUObjectData) * TotalFrameObjects;

    for (i32 FrameIndex = 0;
            FrameIndex < FRAME_OVERLAP;
            ++FrameIndex)
    {
        frame_data * FrameData = GlobalVulkan.FrameData + FrameIndex;
        Assert(ArenaCount < MaxArenas);
        gpu_arena * FrameArena = Arenas + ArenaCount++;
        if (VH_CreateUnAllocArenaBuffer(
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
    }

    VkMemoryRequirements2 TempMemReq = {};
    TempMemReq.sType                     = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2; // sType   sType
    TempMemReq.pNext                     = 0; // Void * pNext
    TempMemReq.memoryRequirements        = {}; // memoryRequirements   memoryRequirements
    i32 TempMemoryTypeIndex = -1;

    // TODO query max dimensions
    VkExtent3D MaxExtent3D = { 1920, 1080, 1};

    VkImageCreateInfo DepthBufferImageInfo = VH_DepthBufferCreateInfo(MaxExtent3D);

    VkDeviceImageMemoryRequirements ReqDepthBuffer;
    ReqDepthBuffer.sType       = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS; // sType   sType
    ReqDepthBuffer.pNext       = 0; // Void * pNext
    ReqDepthBuffer.pCreateInfo = &DepthBufferImageInfo; // VkImageCreateInfo * pCreateInfo
    ReqDepthBuffer.planeAspect = VK_IMAGE_ASPECT_DEPTH_BIT; // planeAspect   planeAspect

    vkGetDeviceImageMemoryRequirements(GlobalVulkan.PrimaryDevice,&ReqDepthBuffer,&TempMemReq);
    VkMemoryPropertyFlags DepthBufferMemoryFlags = VK_MEMORY_GPU;
    TempMemoryTypeIndex = VH_FindSuitableMemoryIndex(GlobalVulkan.PrimaryGPU,TempMemReq,DepthBufferMemoryFlags);

    Assert(ArenaCount < MaxArenas);
    gpu_arena * PrimDepthBufArena = Arenas + ArenaCount++;
    u32 MaxDepthBufferSize = (u32)TempMemReq.memoryRequirements.size;
    if (VH_CreateUnAllocArenaImage(GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice,
                                   MaxDepthBufferSize,
                                   PrimDepthBufArena)
       )
    {
        return 1;
    };
    PrimDepthBufArena->MemoryIndexType = TempMemoryTypeIndex;
    PrimDepthBufArena->Alignment = (u32)TempMemReq.memoryRequirements.alignment;
    GlobalVulkan.PrimaryDepthBufferArena = PrimDepthBufArena;


    // Query default image required memory and allocate
    VkImageUsageFlags UsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB; // VkFormat   format;
    VkImageCreateInfo DefaultTextureInfo = 
        VH_CreateImageCreateInfo2D(MaxExtent3D, Format, UsageFlags);

    VkDeviceImageMemoryRequirements ReqImage;
    ReqImage.sType       = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS; // sType   sType
    ReqImage.pNext       = 0; // Void * pNext
    ReqImage.pCreateInfo = &DefaultTextureInfo; // VkImageCreateInfo * pCreateInfo
    ReqImage.planeAspect = VK_IMAGE_ASPECT_COLOR_BIT; // planeAspect   planeAspect

    vkGetDeviceImageMemoryRequirements(GlobalVulkan.PrimaryDevice,&ReqImage,&TempMemReq);
    VkMemoryPropertyFlags ImageMemoryPropertyFlags = VK_MEMORY_GPU;
    TempMemoryTypeIndex = VH_FindSuitableMemoryIndex(GlobalVulkan.PrimaryGPU,TempMemReq,ImageMemoryPropertyFlags);

    u32 TextureArenaSize = Megabytes(128);
    Assert(ArenaCount < MaxArenas);
    gpu_arena * TextureArena = Arenas + ArenaCount++;
    if (VH_CreateUnAllocArenaImage(GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice,
                                   TextureArenaSize,
                                   TextureArena)
       )
    {
        return 1;
    };
    TextureArena->MemoryIndexType = TempMemoryTypeIndex;
    TextureArena->Alignment = (u32)TempMemReq.memoryRequirements.alignment;
    GlobalVulkan.TextureArena = TextureArena;


    VkDeviceSize TransferbitBufferSize = Megabytes(16);
    u32 SharedBufferFamilyIndexArray[2] = {
        GlobalVulkan.GraphicsQueueFamilyIndex,
        GlobalVulkan.TransferOnlyQueueFamilyIndex
    };

    Assert(ArenaCount < MaxArenas);
    gpu_arena * TransferBitArena = Arenas + ArenaCount++;
    if (VH_CreateUnAllocArenaBuffer(
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

    VkDeviceSize VertexBufferSize = Megabytes(50);

    Assert(ArenaCount < MaxArenas);
    gpu_arena * VertexArena = Arenas + ArenaCount++;
    if (VH_CreateUnAllocArenaBuffer(
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

    VkDeviceSize IndexBufferSize = Megabytes(16);
    Assert(ArenaCount < MaxArenas);
    gpu_arena * IndexArena = Arenas + ArenaCount++;
    if (VH_CreateUnAllocArenaBuffer(
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

    *ArenasCreatedCount = (u32)ArenaCount;

    return 0;
}

i32
CommitArenasToMemory(gpu_arena * Arenas, u32 ArenaCount, device_memory_pools * DeviceMemoryPools)
{
    for (u32 ArenaUnallocIndex = 0;
                ArenaUnallocIndex < ArenaCount;
                ++ArenaUnallocIndex)
    {
        gpu_arena * Arena = Arenas + ArenaUnallocIndex;
        Assert(VK_VALID_HANDLE(Arena->Device));
        Assert(Arena->MaxSize > 0);
        Assert(Arena->MemoryIndexType >= 0 && Arena->MemoryIndexType <= VK_MAX_MEMORY_TYPES);
        Assert( Arena->Type == gpu_arena_type_image || 
                (Arena->Type == gpu_arena_type_buffer && VK_VALID_HANDLE(Arena->Buffer))
            );
        Assert(Arena->CurrentSize == 0);
        Logn("GPU Arena required (%i): %d (Type: %i, Alignment: %i)",ArenaUnallocIndex,Arena->MaxSize, Arena->MemoryIndexType,Arena->Alignment);
        
        i32 MemoryTypeIndex = Arena->MemoryIndexType;
        device_memory_pool * DeviceMemoryPool = DeviceMemoryPools->DeviceMemoryPool + MemoryTypeIndex;
        DeviceMemoryPool->Device                      = Arena->Device; // Device   Device
        DeviceMemoryPool->DeviceMemory                = VK_NULL_HANDLE; // DeviceMemory   DeviceMemory
        DeviceMemoryPool->Size                        += Arena->MaxSize; // Size   Size
    }

    for (i32 MemoryTypeIndex = 0;
                MemoryTypeIndex < VK_MAX_MEMORY_TYPES;
                ++MemoryTypeIndex)
    {
        device_memory_pool * DeviceMemoryPool = DeviceMemoryPools->DeviceMemoryPool + MemoryTypeIndex;
        if (DeviceMemoryPool->Size > 0)
        {
            Logn("Allocating memory pool index %i with size %i",MemoryTypeIndex, DeviceMemoryPool->Size);
            VkMemoryAllocateInfo AllocateInfo;

            AllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; // VkStructureType   sType;
            AllocateInfo.pNext           = 0;                       // Void * pNext;
            AllocateInfo.allocationSize  = DeviceMemoryPool->Size;             // VkDeviceSize allocationSize;
            AllocateInfo.memoryTypeIndex = MemoryTypeIndex; // u32_t memoryTypeIndex;

            if (VK_FAILS(vkAllocateMemory(DeviceMemoryPool->Device, &AllocateInfo, 0, &DeviceMemoryPool->DeviceMemory)))
            {
                return 1;
            }

            u32 SizeAllocated = 0;
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
                    Assert(((SizeAllocated + Arena->Alignment - 1) & ~(Arena->Alignment - 1)) == SizeAllocated);
                    if (Arena->Type == gpu_arena_type_buffer)
                    {
                        Logn("Binding arena %i to pool %i (offset: %u)",ArenaUnallocIndex,MemoryTypeIndex,SizeAllocated);
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

            QUEUE_DELETE_MEMORY_POOL(&PrimaryVulkanDestroyQueue, DeviceMemoryPool);
        }
    }

    return 0;
}


// ivp -- initialize vulkan procedure
i32
InitializeVulkan(i32 Width, i32 Height, 
                 graphics_platform_window PlatformWindow,
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
    QUEUE_DELETE_INSTANCE(&PrimaryVulkanDestroyQueue,&GlobalVulkan.Instance);

    if (PlatformWindow.pfnVulkanCreateSurface(PlatformWindow.SurfaceData,pfnOSSurface, GlobalVulkan.Instance, &GlobalVulkan.Surface)) return 1;
    QUEUE_DELETE_SURFACEKHR(&PrimaryVulkanDestroyQueue,GlobalVulkan.Instance,&GlobalVulkan.Surface);

    if (VulkanGetPhysicalDevice()) return 1;
    if (VulkanCreateLogicaDevice()) return 1;
    QUEUE_DELETE_DEVICE(&PrimaryVulkanDestroyQueue,&GlobalVulkan.PrimaryDevice);

    if (VulkanCreateSwapChain(Width, Height)) return 1;
    QUEUE_DELETE_SWAPCHAINKHR(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&GlobalVulkan.Swapchain);

    CreateUnallocatedMemoryArenas(GlobalVulkan.MemoryArenas,
                                  ArrayCount(GlobalVulkan.MemoryArenas),
                                  &GlobalVulkan.MemoryArenaCount);

    for (u32 i = 0; i < GlobalVulkan.MemoryArenaCount;++i)
    {
        gpu_arena * Arena = GlobalVulkan.MemoryArenas + i;
        QUEUE_DELETE_ARENA(&PrimaryVulkanDestroyQueue,Arena);
    }

    CommitArenasToMemory(&GlobalVulkan.MemoryArenas[0],
                         GlobalVulkan.MemoryArenaCount,
                         &GlobalVulkan.DeviceMemoryPools);

    /* INIT DESCRIPTOR SETS */
    /* Level 1 */ CreateDescriptorSetLayoutGlobal();
    QUEUE_DELETE_DESCRIPTORSETLAYOUT(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&GlobalVulkan._GlobalSetLayout);
    /* Level 2 */ CreateDescriptorSetLayoutObjects();
    QUEUE_DELETE_DESCRIPTORSETLAYOUT(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&GlobalVulkan._ObjectsSetLayout);
    /* Level 3 */ CreateDescriptorSetTextures();
    QUEUE_DELETE_DESCRIPTORSETLAYOUT(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&GlobalVulkan._DebugTextureSetLayout);

    VkDeviceSize TextureBufferSize = Megabytes(15);
    VkDescriptorPoolSize DescriptorPoolSizes[] =
	{
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
	};

	VkDescriptorPoolCreateInfo PoolInfo = {};
	PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	PoolInfo.flags = 0;
	PoolInfo.maxSets = 10;
	PoolInfo.poolSizeCount = ArrayCount(DescriptorPoolSizes);
	PoolInfo.pPoolSizes = DescriptorPoolSizes;

	vkCreateDescriptorPool(GlobalVulkan.PrimaryDevice, &PoolInfo, nullptr, &GlobalVulkan._DescriptorPool);
    QUEUE_DELETE_DESCRIPTORPOOL(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&GlobalVulkan._DescriptorPool);


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

        if (VH_CreateCommandBuffers(
                    GlobalVulkan.PrimaryDevice,
                    FrameData->CommandPool,
                    1,
                    &FrameData->PrimaryCommandBuffer)) return 1;

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
        frame_data * FrameData = GlobalVulkan.FrameData + FrameIndex;
        QUEUE_DELETE_COMMANDPOOL(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&FrameData->CommandPool);
        QUEUE_DELETE_FENCE(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &FrameData->RenderFence);
        QUEUE_DELETE_SEMAPHORE(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &FrameData->ImageAvailableSemaphore);
        QUEUE_DELETE_SEMAPHORE(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &FrameData->RenderSemaphore);
    }

    // INIT PIPELINES
    CreatePipelineLayoutTexture(GlobalVulkan.PipelineLayout + 0);
    QUEUE_DELETE_PIPELINELAYOUT(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &GlobalVulkan.PipelineLayout[0]);

    // Staging buffer commands
    if (VH_CreateCommandPool(
                GlobalVulkan.PrimaryDevice,
                GlobalVulkan.TransferOnlyQueueFamilyIndex,
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                &GlobalVulkan.CommandPoolTransferBit
                )) return 1;

    QUEUE_DELETE_COMMANDPOOL(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&GlobalVulkan.CommandPoolTransferBit);

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

    if (VulkanInitDefaultRenderPass()) return 1;
    QUEUE_DELETE_RENDERPASS(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &GlobalVulkan.RenderPass);

    if (VulkanInitFramebuffers()) return 1;
    for (u32 ImageIndex = 0;
                ImageIndex < GlobalVulkan.SwapchainImageCount;
                ++ImageIndex)
    {
        QUEUE_DELETE_FRAMEBUFFER(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &GlobalVulkan.Framebuffers[ImageIndex]);
        QUEUE_DELETE_IMAGEVIEW(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &GlobalVulkan.SwapchainImageViews[ImageIndex]);
    }

    // As for now a single sampler for all textures
    VkSamplerCreateInfo SamplerCreateInfo = VulkanSamplerCreateInfo(VK_FILTER_LINEAR,VK_SAMPLER_ADDRESS_MODE_REPEAT);

    VK_CHECK(vkCreateSampler(GlobalVulkan.PrimaryDevice,&SamplerCreateInfo,0,&GlobalVulkan.TextureSampler));


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
CleanVulkanObjectsQueue(vulkan_destroy_queue * Queue)
{
    const char * Names[] = 
    {
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

            "vulkan_destructor_type_vkDestroyArenaCustom",
            "vulkan_destructor_type_vkDestroyMemoryPoolCustom"
    };

    for (i32 i = (Queue->ItemsCount - 1); i >= 0;--i)
    {
       vulkan_destroy_queue_item * Item = Queue->Items + i;
       Logn("Deleting global queue %s",Names[Item->DestructorType]); 
       switch (Item->DestructorType)
       {
           // CUSTOM DESTRUCTORS
           case vulkan_destructor_type_vkDestroyMemoryPoolCustom:
           {
               device_memory_pool * Pool = (device_memory_pool *)Item->Params[0];
               vkFreeMemory(Pool->Device, Pool->DeviceMemory, 0);
           } break;
           case vulkan_destructor_type_vkDestroyArenaCustom:
           {
               gpu_arena * Arena = (gpu_arena *)Item->Params[0];
               if (Arena->Type == gpu_arena_type_buffer)
               {
                   if (VK_VALID_HANDLE(Arena->Buffer))
                   {
                       vkDestroyBuffer(Arena->Device,Arena->Buffer,0);
                   } 
               }
               else
               {
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
               Arena->WriteToAddr     = 0; // Void * WriteToAddr
           } break;


            // GENERIC DESTRUCTOR
           case vulkan_destructor_type_vkDestroySurfaceKHR:
               {
                   VkInstance instance = (VkInstance)Item->Params[0];
                   Assert(VK_VALID_HANDLE(instance));
                   VkSurfaceKHR * surface = (VkSurfaceKHR *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*surface));
                   vkDestroySurfaceKHR(instance,*surface,0);
                   *surface = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroySwapchainKHR:
               {
                   VkDevice device = (VkDevice)Item->Params[0];
                   Assert(VK_VALID_HANDLE(device));
                   VkSwapchainKHR * swapchain = (VkSwapchainKHR *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*swapchain));
                   vkDestroySwapchainKHR(device,*swapchain,0);
                   *swapchain = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroyInstance:
               {
                   VkInstance * instance = (VkInstance *)Item->Params[0];
                   Assert(VK_VALID_HANDLE(*instance));
                   vkDestroyInstance(*instance,0);
                   *instance = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroyCommandPool:
               {
                   VkDevice device = (VkDevice)Item->Params[0];
                   Assert(VK_VALID_HANDLE(device));
                   VkCommandPool * commandPool = (VkCommandPool *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*commandPool));
                   vkDestroyCommandPool(device,*commandPool,0);
                   *commandPool = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroyShaderModule:
               {
                   VkDevice device = (VkDevice)Item->Params[0];
                   Assert(VK_VALID_HANDLE(device));
                   VkShaderModule * shaderModule = (VkShaderModule *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*shaderModule));
                   vkDestroyShaderModule(device,*shaderModule,0);
                   *shaderModule = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroyDevice:
               {
                   VkDevice * device = (VkDevice *)Item->Params[0];
                   Assert(VK_VALID_HANDLE(*device));
                   vkDestroyDevice(*device,0);
                   *device = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroyFence:
               {
                   VkDevice device = (VkDevice)Item->Params[0];
                   Assert(VK_VALID_HANDLE(device));
                   VkFence * fence = (VkFence *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*fence));
                   vkDestroyFence(device,*fence,0);
                   *fence = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroySemaphore:
               {
                   VkDevice device = (VkDevice)Item->Params[0];
                   Assert(VK_VALID_HANDLE(device));
                   VkSemaphore * semaphore = (VkSemaphore *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*semaphore));
                   vkDestroySemaphore(device,*semaphore,0);
                   *semaphore = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroyRenderPass:
               {
                   VkDevice device = (VkDevice)Item->Params[0];
                   Assert(VK_VALID_HANDLE(device));
                   VkRenderPass * renderPass = (VkRenderPass *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*renderPass));
                   vkDestroyRenderPass(device,*renderPass,0);
                   *renderPass = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroySampler:
               {
                   VkDevice device = (VkDevice)Item->Params[0];
                   Assert(VK_VALID_HANDLE(device));
                   VkSampler * sampler = (VkSampler *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*sampler));
                   vkDestroySampler(device,*sampler,0);
                   *sampler = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroyImageView:
               {
                   VkDevice device = (VkDevice)Item->Params[0];
                   Assert(VK_VALID_HANDLE(device));
                   VkImageView * imageView = (VkImageView *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*imageView));
                   vkDestroyImageView(device,*imageView,0);
                   *imageView = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroyFramebuffer:
               {
                   VkDevice device = (VkDevice)Item->Params[0];
                   Assert(VK_VALID_HANDLE(device));
                   VkFramebuffer * framebuffer = (VkFramebuffer *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*framebuffer));
                   vkDestroyFramebuffer(device,*framebuffer,0);
                   *framebuffer = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroyPipeline:
               {
                   VkDevice device = (VkDevice)Item->Params[0];
                   Assert(VK_VALID_HANDLE(device));
                   VkPipeline * pipeline = (VkPipeline *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*pipeline));
                   vkDestroyPipeline(device,*pipeline,0);
                   *pipeline = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroyPipelineLayout:
               {
                   VkDevice device = (VkDevice)Item->Params[0];
                   Assert(VK_VALID_HANDLE(device));
                   VkPipelineLayout * pipelineLayout = (VkPipelineLayout *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*pipelineLayout));
                   vkDestroyPipelineLayout(device,*pipelineLayout,0);
                   *pipelineLayout = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroyDescriptorSetLayout:
               {
                   VkDevice device = (VkDevice)Item->Params[0];
                   Assert(VK_VALID_HANDLE(device));
                   VkDescriptorSetLayout * descriptorSetLayout = (VkDescriptorSetLayout *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*descriptorSetLayout));
                   vkDestroyDescriptorSetLayout(device,*descriptorSetLayout,0);
                   *descriptorSetLayout = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroyDescriptorPool:
               {
                   VkDevice device = (VkDevice)Item->Params[0];
                   Assert(VK_VALID_HANDLE(device));
                   VkDescriptorPool * descriptorPool = (VkDescriptorPool *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*descriptorPool));
                   vkDestroyDescriptorPool(device,*descriptorPool,0);
                   *descriptorPool = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroyBuffer:
               {
                   VkDevice device = (VkDevice)Item->Params[0];
                   Assert(VK_VALID_HANDLE(device));
                   VkBuffer * buffer = (VkBuffer *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*buffer));
                   vkDestroyBuffer(device,*buffer,0);
                   *buffer = VK_NULL_HANDLE;
               } break;
           case vulkan_destructor_type_vkDestroyImage:
               {
                   VkDevice device = (VkDevice)Item->Params[0];
                   Assert(VK_VALID_HANDLE(device));
                   VkImage * image = (VkImage *)Item->Params[1];
                   Assert(VK_VALID_HANDLE(*image));
                   //Logn("Image: %p", *image);
                   vkDestroyImage(device,*image,0);
                   *image = VK_NULL_HANDLE;
               } break;
       }
       Queue->ItemsCount -= 1;
    }
}

void
CloseVulkan()
{
    VulkanWaitForDevices();

    /* ALWAYS AFTER WAIT IDLE */
    if (VK_VALID_HANDLE(GlobalVulkan.TextureSampler))
    {
        vkDestroySampler(GlobalVulkan.PrimaryDevice,GlobalVulkan.TextureSampler,0);
    }

    RenderFreeShaders();

    CleanVulkanObjectsQueue(&PrimaryVulkanDestroyQueue);
}

GRAPHICS_WAIT_FOR_RENDER(WaitForRender)
{
    if (GlobalWindowIsMinimized) return 0;

    VK_CHECK(vkWaitForFences(GlobalVulkan.PrimaryDevice, 1, &GetCurrentFrame()->RenderFence, true, 1000000000));
    VK_CHECK(vkResetFences(GlobalVulkan.PrimaryDevice, 1, &GetCurrentFrame()->RenderFence));

    return 0;
}


