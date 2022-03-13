#include "vulkan_local.h"

i32
GroundPipelineBuildDensities(file_contents * Files)
{
    ground_info * Ground = &GlobalVulkan.GroundInfo;

    VkDescriptorSetLayoutBinding LayoutBinding = 
        VH_CreateDescriptorSetLayoutBinding(0,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                VK_SHADER_STAGE_COMPUTE_BIT);

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

    VK_CHECK(vkCreateDescriptorSetLayout(GlobalVulkan.PrimaryDevice,&SetInfo,nullptr,&GlobalVulkan.GroundInfo.SetLayoutDensityVolume));

    // BUILD POOL
    VkDescriptorPoolSize DescriptorPoolSizes[] =
	{
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 }
	};

	VkDescriptorPoolCreateInfo PoolInfo = {};
	PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	PoolInfo.flags = 0;
	PoolInfo.maxSets = 10;
	PoolInfo.poolSizeCount = ArrayCount(DescriptorPoolSizes);
	PoolInfo.pPoolSizes = DescriptorPoolSizes;

	vkCreateDescriptorPool(GlobalVulkan.PrimaryDevice, &PoolInfo, nullptr, &Ground->DescriptorPool);
    


    return 0;
}

// gg (ground generation)

struct ground_voxel_LOD
{
   /*0 */r32 VoxelDim;                     // 65;
   /*4 */r32 VoxelDimMinusOne;             // 64;
   /*8 */v2 wsVoxelSize;                   // v2(1.0/64.0, 0); // could be 1/63, 1/31, 1/15 depending on LOD
   /*16 */v2 wsChunkSize;                  // 4. 0;
   /*24 */v2 InvVoxelDim;                   // v2(1.0/65.0, 0);
   /*32 */v2 InvVoxelDimMinusOne;           // v2(1.0/64.0, 0);
   /*36 */r32 Margin;                       // 4;
   /*40 */r32 VoxelDimPlusMargins;          // 73;
   /*44 */r32 VoxelDimPlusMarginsMinusOne;  // 72;
   r32 _Padding01;
   /*48 */v2 InvVoxelDimPlusMargins;        // v2(1.0/73.0, 0);
   /*56 */v2 InvVoxelDimPlusMarginsMinusOne;// v2(1.0/72.0, 0);
};


i32
GroundCreateLayoutGlobals()
{
    VkDescriptorSetLayoutBinding LayoutBindingViewPort = 
        VH_CreateDescriptorSetLayoutBinding(0,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                VK_SHADER_STAGE_VERTEX_BIT);

    VkDescriptorSetLayoutBinding LayoutBindingLOD = 
        VH_CreateDescriptorSetLayoutBinding(1,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                VK_SHADER_STAGE_VERTEX_BIT);

    VkDescriptorSetLayoutBinding DescriptorSetLayoutBindings[] = 
    {
        LayoutBindingViewPort,
        LayoutBindingLOD
    };

    VkDescriptorSetLayoutCreateInfo SetInfo;
    SetInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO; // VkStructureType   sType;
    SetInfo.pNext        = 0; // Void * pNext;
    SetInfo.flags        = 0; // VkDescriptorSetLayoutCreateFlags   flags;
    SetInfo.bindingCount = ArrayCount(DescriptorSetLayoutBindings); // uint32_t   bindingCount;
    SetInfo.pBindings    = &DescriptorSetLayoutBindings[0]; // Typedef * pBindings;

    VK_CHECK(vkCreateDescriptorSetLayout(GlobalVulkan.PrimaryDevice,&SetInfo,0,&GlobalVulkan.GroundInfo.SetLayoutGlobals));

    return 0;
}

struct chunk_data
{
    v3  wsChunkPos; //wsCoord of lower-left corner
    r32 opacity;
};

i32
GroundCreateLayoutsChunks()
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

    VK_CHECK(vkCreateDescriptorSetLayout(GlobalVulkan.PrimaryDevice,&SetInfo,nullptr,&GlobalVulkan.GroundInfo.SetLayoutChunks));

    return 0;
}



struct ground_vertex_shader_input
{
    v3 Position;
    v2 UV;
};

struct ground_vertex_inputs_description
{
    VkVertexInputBindingDescription   Bindings[1];
    VkVertexInputAttributeDescription Attributes[2];
};


i32
GroundCreateDensityVolumePass(VkPhysicalDevice PhysicalDevice,VkDevice Device)
{
    ground_density_volume_pass * VolumePass = &GlobalVulkan.GroundInfo.DensityVolumePass;

    VolumePass->x = 33;
    VolumePass->y = 33;
    VolumePass->z = 33;

    vulkan_image * Color = &VolumePass->Color;
    VkSampler * Sampler = &VolumePass->Sampler;

    VkImageTiling ImageTiling;
    ImageTiling = VK_IMAGE_TILING_OPTIMAL;
    Color->Format = VK_FORMAT_R32_SFLOAT;

    VkImageUsageFlags Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
                              VK_IMAGE_USAGE_SAMPLED_BIT;

    VkExtent3D Extent;
    Extent.width  = 33; // uint32_t   width;
    Extent.height = 33; // uint32_t   height;
    Extent.depth  = 33; // uint32_t   depth;

    VkImageCreateInfo ImageCreateInfo = {};

    ImageCreateInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO; // VkStructureType   sType;
    ImageCreateInfo.pNext                 = 0;                         // Void * pNext;
    ImageCreateInfo.flags                 = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;                         // VkImageCreateFlags flags;
    ImageCreateInfo.imageType             = VK_IMAGE_TYPE_3D;          // VkImageType imageType;
    ImageCreateInfo.format                = Color->Format;                    // VkFormat format;
    ImageCreateInfo.extent                = Extent;                    // VkExtent3D extent;
    ImageCreateInfo.mipLevels             = 1;                         // u32_t mipLevels;
    ImageCreateInfo.arrayLayers           = 1;                         // u32_t arrayLayers;
    ImageCreateInfo.pQueueFamilyIndices   = 0;
    ImageCreateInfo.samples               = VK_SAMPLE_COUNT_1_BIT;     // VkSampleCountFlagBits samples;
    ImageCreateInfo.tiling                = ImageTiling;   // VkImageTiling tiling;

    ImageCreateInfo.usage                 = Usage;

    ImageCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE; // VkSharingMode sharingMode;
    ImageCreateInfo.queueFamilyIndexCount = 0;                         // u32_t queueFamilyIndexCount;
    ImageCreateInfo.pQueueFamilyIndices   = 0;                         // Typedef * pQueueFamilyIndices;
    ImageCreateInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED; // VkImageLayout initialLayout;

    VK_CHECK(vkCreateImage(Device,&ImageCreateInfo, 0, &Color->Image));

    VkMemoryRequirements MemReq;
    vkGetImageMemoryRequirements(Device, Color->Image, &MemReq);
    VkMemoryPropertyFlags PropertyFlags = VK_MEMORY_GPU;

    u32 MemoryTypeIndex = VH_FindSuitableMemoryIndex(PhysicalDevice,MemReq,PropertyFlags);
    if (MemoryTypeIndex < 0)
    {
        Log("Couldn't find suitable CPU-GPU memory index\n");
        return 1;
    }

    VkMemoryAllocateInfo AllocateInfo;

    AllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; // VkStructureType   sType;
    AllocateInfo.pNext           = 0;                       // Void * pNext;
    AllocateInfo.allocationSize  = MemReq.size;             // VkDeviceSize allocationSize;
    AllocateInfo.memoryTypeIndex = (u32)MemoryTypeIndex; // u32_t memoryTypeIndex;

    VK_CHECK(vkAllocateMemory(Device, &AllocateInfo, 0, &Color->DeviceMemory));

    vkBindImageMemory(Device,Color->Image,Color->DeviceMemory,0);

    VkImageSubresourceRange SubresourceRange;
    SubresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT; // VkImageAspectFlags   aspectMask;
    SubresourceRange.baseMipLevel   = 0; // u32_t   baseMipLevel;
    SubresourceRange.levelCount     = 1; // u32_t   levelCount;
    SubresourceRange.baseArrayLayer = 0; // u32_t   baseArrayLayer;
    SubresourceRange.layerCount     = VolumePass->z; // u32_t   layerCount;

    VkImageViewCreateInfo ImageViewCreateInfo = {}; 

    ImageViewCreateInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; // VkStructureType   sType;
    ImageViewCreateInfo.pNext    = 0;                     // Void * pNext;
    ImageViewCreateInfo.flags    = 0;                     // VkImageViewCreateFlags flags;
    ImageViewCreateInfo.image    = Color->Image;                 // VkImage image;
    ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY; // VkImageViewType viewType;
    ImageViewCreateInfo.format   = Color->Format;                // VkFormat format;
    //ImageViewCreateInfo.components       = ; // VkComponentMapping   components;
    ImageViewCreateInfo.subresourceRange = SubresourceRange; // VkImageSubresourceRange   subresourceRange;

    VK_CHECK(vkCreateImageView( Device, &ImageViewCreateInfo, 0, &Color->ImageView ));

    VkSamplerCreateInfo SamplerInfo = {};
    SamplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO; // VkStructureType   sType;
    SamplerInfo.magFilter               = VK_FILTER_LINEAR; // VkFilter   magFilter;
    SamplerInfo.minFilter               = VK_FILTER_LINEAR; // VkFilter   minFilter;
    SamplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR; // VkSamplerMipmapMode   mipmapMode;
    SamplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // VkSamplerAddressMode   addressModeU;
    SamplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // VkSamplerAddressMode   addressModeV;
    SamplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // VkSamplerAddressMode   addressModeW;
    SamplerInfo.mipLodBias              = 0.0f; // FLOAT   mipLodBias;
    SamplerInfo.anisotropyEnable        = VK_FALSE; // VkBool32   anisotropyEnable;
    SamplerInfo.maxAnisotropy           = 1.0f; // FLOAT   maxAnisotropy;
    SamplerInfo.compareEnable           = VK_FALSE; // VkBool32   compareEnable;
    SamplerInfo.minLod                  = 0.0f; // FLOAT   minLod;
    SamplerInfo.maxLod                  = 1.0f; // FLOAT   maxLod;
    SamplerInfo.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE; // VkBorderColor   borderColor;
    SamplerInfo.unnormalizedCoordinates = VK_FALSE; // VkBool32   unnormalizedCoordinates;

    VK_CHECK(vkCreateSampler(Device, &SamplerInfo, nullptr, Sampler));


    VkAttachmentDescription AttachDesc;

    AttachDesc.flags          = 0; // VkAttachmentDescriptionFlags   flags;
    AttachDesc.format         = Color->Format; // VkFormat   format;
    AttachDesc.samples        = VK_SAMPLE_COUNT_1_BIT; // VkSampleCountFlagBits   samples;
    AttachDesc.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR; // VkAttachmentLoadOp   loadOp;
    AttachDesc.storeOp        = VK_ATTACHMENT_STORE_OP_STORE; // VkAttachmentStoreOp   storeOp;
    AttachDesc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // VkAttachmentLoadOp   stencilLoadOp;
    AttachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // VkAttachmentStoreOp   stencilStoreOp;
    AttachDesc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED; // VkImageLayout   initialLayout;
    AttachDesc.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // VkImageLayout   finalLayout;

    VkAttachmentReference ColorRef = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription SubpassDescription;

    SubpassDescription.flags                   = 0; // VkSubpassDescriptionFlags   flags;
    SubpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS; // VkPipelineBindPoint   pipelineBindPoint;
    SubpassDescription.inputAttachmentCount    = 0; // uint32_t   inputAttachmentCount;
    SubpassDescription.pInputAttachments       = 0; // Typedef * pInputAttachments;
    SubpassDescription.colorAttachmentCount    = 1; // uint32_t   colorAttachmentCount;
    SubpassDescription.pColorAttachments       = &ColorRef; // Typedef * pColorAttachments;
    SubpassDescription.pResolveAttachments     = 0; // Typedef * pResolveAttachments;
    SubpassDescription.pDepthStencilAttachment = 0; // Typedef * pDepthStencilAttachment;
    SubpassDescription.preserveAttachmentCount = 0; // uint32_t   preserveAttachmentCount;
    SubpassDescription.pPreserveAttachments    = 0; // Typedef * pPreserveAttachments;

    VkSubpassDependency SubpassDependencyFromReadToColor;
    SubpassDependencyFromReadToColor.srcSubpass      = VK_SUBPASS_EXTERNAL; // uint32_t   srcSubpass;
    SubpassDependencyFromReadToColor.dstSubpass      = 0; // uint32_t   dstSubpass;
    SubpassDependencyFromReadToColor.srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // VkPipelineStageFlags   srcStageMask;
    SubpassDependencyFromReadToColor.dstStageMask    = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR; // VkPipelineStageFlags   dstStageMask;
    SubpassDependencyFromReadToColor.srcAccessMask   = VK_ACCESS_SHADER_READ_BIT; // VkAccessFlags   srcAccessMask;
    SubpassDependencyFromReadToColor.dstAccessMask   = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR; // VkAccessFlags   dstAccessMask;
    SubpassDependencyFromReadToColor.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; // VkDependencyFlags   dependencyFlags;

    VkSubpassDependency SubpassDependencyFromColorToRead;
    SubpassDependencyFromColorToRead.srcSubpass      = 0; // uint32_t   srcSubpass;
    SubpassDependencyFromColorToRead.dstSubpass      = VK_SUBPASS_EXTERNAL; // uint32_t   dstSubpass;
    SubpassDependencyFromColorToRead.srcStageMask    = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR; // VkPipelineStageFlags   srcStageMask;
    SubpassDependencyFromColorToRead.dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // VkPipelineStageFlags   dstStageMask;
    SubpassDependencyFromColorToRead.srcAccessMask   = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR; // VkAccessFlags   srcAccessMask;
    SubpassDependencyFromColorToRead.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT; // VkAccessFlags   dstAccessMask;
    SubpassDependencyFromColorToRead.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; // VkDependencyFlags   dependencyFlags;

    VkSubpassDependency SubpassDependency[] =
    {
        SubpassDependencyFromReadToColor,
        SubpassDependencyFromColorToRead
    };

    VkRenderPassCreateInfo RenderPassCreateInfo;

    RenderPassCreateInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO; // VkStructureType   sType;
    RenderPassCreateInfo.pNext           = 0; // Void * pNext;
    RenderPassCreateInfo.flags           = 0; // VkRenderPassCreateFlags   flags;
    RenderPassCreateInfo.attachmentCount = 1; // uint32_t   attachmentCount;
    RenderPassCreateInfo.pAttachments    = &AttachDesc; // Typedef * pAttachments;
    RenderPassCreateInfo.subpassCount    = 1; // uint32_t   subpassCount;
    RenderPassCreateInfo.pSubpasses      = &SubpassDescription; // Typedef * pSubpasses;
    RenderPassCreateInfo.dependencyCount = ArrayCount(SubpassDependency); // uint32_t   dependencyCount;
    RenderPassCreateInfo.pDependencies   = SubpassDependency; // Typedef * pDependencies;

    VK_CHECK(vkCreateRenderPass(Device,&RenderPassCreateInfo, 0, &VolumePass->Renderpass));

    VkFramebufferCreateInfo FbCreateInfo;
    FbCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO; // VkStructureType   sType;
    FbCreateInfo.pNext           = 0; // Void * pNext;
    FbCreateInfo.flags           = 0; // VkFramebufferCreateFlags   flags;
    FbCreateInfo.renderPass      = VolumePass->Renderpass; // VkRenderPass   renderPass;
    FbCreateInfo.attachmentCount = 1; // uint32_t   attachmentCount;
    FbCreateInfo.pAttachments    = &Color->ImageView; // Typedef * pAttachments;
    FbCreateInfo.width           = VolumePass->x; // uint32_t   width;
    FbCreateInfo.height          = VolumePass->y; // uint32_t   height;
    FbCreateInfo.layers          = VolumePass->z; // uint32_t   layers;


    VK_CHECK( vkCreateFramebuffer(Device, &FbCreateInfo, 0, &VolumePass->Framebuffer));

    VolumePass->Descriptor.imageView = Color->ImageView;
    VolumePass->Descriptor.sampler = VolumePass->Sampler;
    VolumePass->Descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    return 0;
}

ground_vertex_inputs_description
GroundGetVertexInputsDescription()
{
    ground_vertex_inputs_description InputsDescription;

    VkVertexInputBindingDescription Binding;

    Binding.binding   = 0;                           // u32_t binding;
    Binding.stride    = sizeof(ground_vertex_shader_input);        // u32_t stride;
    Binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // VkVertexInputRate inputRate;

    VkVertexInputAttributeDescription * Attribute = (InputsDescription.Attributes + 0);

    Attribute->location = 0;                          // u32_t location;
    Attribute->binding  = 0;                          // u32_t binding;
    Attribute->format   = VK_FORMAT_R32G32B32_SFLOAT; // VkFormat format;
    Attribute->offset   = offsetof(vertex_point,P);   // u32_t offset;

    Attribute = (InputsDescription.Attributes + 1);

    Attribute->location = 1;                             // u32_t location;
    Attribute->binding  = 0;                             // u32_t binding;
    Attribute->format   = VK_FORMAT_R32G32_SFLOAT; // VkFormat format;
    Attribute->offset   = offsetof(ground_vertex_shader_input,UV);  // u32_t offset;

    InputsDescription.Bindings[0] = Binding; // CONSTANTARRAY   Bindings;

    return InputsDescription;
}

struct ground_push_constant
{
    m4 MVP;
};

i32
GroundCreatePipelineBuildDensities()
{
    ground_info * Ground = &GlobalVulkan.GroundInfo;

    VkDescriptorSetLayout LayoutSets[] = 
    {
        Ground->SetLayoutGlobals,
        Ground->SetLayoutChunks
    };

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = VH_CreatePipelineLayoutCreateInfo();

#if 0
    PipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    PipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
#else
    VkPushConstantRange PushConstant;
    PushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // VkShaderStageFlags   stageFlags;
    PushConstant.offset     = 0; // u32_t   offset;
    PushConstant.size       = sizeof(ground_push_constant); // u32_t   size;

    PipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    PipelineLayoutCreateInfo.pPushConstantRanges = &PushConstant;
#endif

    PipelineLayoutCreateInfo.setLayoutCount = ArrayCount(LayoutSets);
    PipelineLayoutCreateInfo.pSetLayouts = LayoutSets;

    VK_CHECK(vkCreatePipelineLayout(GlobalVulkan.PrimaryDevice, &PipelineLayoutCreateInfo, 0, &Ground->PipelineLayoutBuildDensities));

    VkShaderModule VertexShader = GlobalVulkan.GroundInfo.ShaderVertexBuildDensity;
    VkShaderModule GeometryShader = GlobalVulkan.GroundInfo.ShaderGeometryBuildDensity;
    VkShaderModule FragmentShader = GlobalVulkan.GroundInfo.ShaderFragmentBuildDensity;

    vulkan_pipeline VulkanPipeline;

    VulkanPipeline.ShaderStagesCount = 3; // u32 ShaderStagesCount;
    VulkanPipeline.ShaderStages[0]   = VH_CreateShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT,VertexShader);
    VulkanPipeline.ShaderStages[1]   = VH_CreateShaderStageInfo(VK_SHADER_STAGE_GEOMETRY_BIT,GeometryShader);
    VulkanPipeline.ShaderStages[2]   = VH_CreateShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT,FragmentShader);

    VulkanPipeline.VertexInputInfo      = VH_CreateVertexInputStateInfo(); // VkPipelineVertexInputStateCreateInfo   VertexInputInfo;

    ground_vertex_inputs_description VertexInputDesc = GroundGetVertexInputsDescription();
    VulkanPipeline.VertexInputInfo.vertexBindingDescriptionCount   = 1;                              // u32_t vertexBindingDescriptionCount;
    VulkanPipeline.VertexInputInfo.pVertexBindingDescriptions      = &VertexInputDesc.Bindings[0];   // Typedef * pVertexBindingDescriptions;
    VulkanPipeline.VertexInputInfo.vertexAttributeDescriptionCount = ArrayCount(VertexInputDesc.Attributes);                              // u32_t vertexAttributeDescriptionCount;
    VulkanPipeline.VertexInputInfo.pVertexAttributeDescriptions    = &VertexInputDesc.Attributes[0]; // Typedef * pVertexAttributeDescriptions;


    VkPipelineDepthStencilStateCreateInfo DepthStencil = {};

    DepthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO; // VkStructureType   sType;
    DepthStencil.pNext                 = 0;                           // Void * pNext;
    DepthStencil.flags                 = 0;                           // VkPipelineDepthStencilStateCreateFlags flags;
    DepthStencil.depthTestEnable       = VK_FALSE;                     // VkBool32 depthTestEnable;
    DepthStencil.depthWriteEnable      = VK_FALSE;                     // VkBool32 depthWriteEnable;
    DepthStencil.depthCompareOp        = VK_COMPARE_OP_ALWAYS; // VkCompareOp depthCompareOp;
    DepthStencil.depthBoundsTestEnable = VK_FALSE;                    // VkBool32 depthBoundsTestEnable;
    DepthStencil.stencilTestEnable     = VK_FALSE;                    // VkBool32 stencilTestEnable;
    //DepthStencil.front               = ; // VkStencilOpState front;
    //DepthStencil.back                = ; // VkStencilOpState back;
    DepthStencil.minDepthBounds        = 0.0f;                        // FLOAT minDepthBounds;
    DepthStencil.maxDepthBounds        = 1.0f;                        // FLOAT maxDepthBounds;
    
    VulkanPipeline.DepthStencil = DepthStencil;

    VulkanPipeline.InputAssembly        = 
        VH_CreatePipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST); // VkPipelineInputAssemblyStateCreateInfo   InputAssembly;

    VulkanPipeline.Viewport             = VH_CreateDefaultViewport(GlobalVulkan.WindowExtension); // VkViewport   Viewport;

    VulkanPipeline.Scissor.offset = {0,0};
    VulkanPipeline.Scissor.extent = GlobalVulkan.WindowExtension;

    VulkanPipeline.Rasterizer           = 
        VH_CreatePipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL); // VkPipelineRasterizationStateCreateInfo   Rasterizer;

    VulkanPipeline.ColorBlendAttachment = VH_CreatePipelineColorBlendAttachmentState();  // VkPipelineColorBlendAttachmentState ColorBlendAttachment;
    VulkanPipeline.Multisampling        = VH_CreatePipelineMultisampleStateCreateInfo(); // VkPipelineMultisampleStateCreateInfo Multisampling;
    VulkanPipeline.PipelineLayout       = GlobalVulkan.GroundInfo.PipelineLayoutBuildDensities;                      // VkPipelineLayout PipelineLayout;

    VkPipeline Pipeline = VH_PipelineBuilder(&VulkanPipeline, GlobalVulkan.PrimaryDevice, Ground->DensityVolumePass.Renderpass);

    if (VK_VALID_HANDLE(Pipeline))
    {
        VulkanPipeline.Pipeline = Pipeline;
        Ground->PipelineBuildDensities = VulkanPipeline;
    }

    return 0;
}


i32
GroundInitPipelines(file_contents * Files, u32 FileCount)
{
    ground_info * Ground = &GlobalVulkan.GroundInfo;

    // BUILD LAYOUTS
    GroundCreateLayoutGlobals();
    GroundCreateLayoutsChunks();

    // BUILD POOL
    VkDescriptorPoolSize DescriptorPoolSizes[] =
	{
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 }
	};

	VkDescriptorPoolCreateInfo PoolInfo = {};
	PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	PoolInfo.flags = 0;
	PoolInfo.maxSets = 10;
	PoolInfo.poolSizeCount = ArrayCount(DescriptorPoolSizes);
	PoolInfo.pPoolSizes = DescriptorPoolSizes;

	vkCreateDescriptorPool(GlobalVulkan.PrimaryDevice, &PoolInfo, nullptr, &Ground->DescriptorPool);

    // Globals 
    VkDeviceSize GlobalBufferSize = VH_PaddedUniformBuffer(sizeof(v4)) + VH_PaddedUniformBuffer(sizeof(ground_voxel_LOD));
    if (VH_CreateBuffer(GlobalVulkan.PrimaryGPU, GlobalVulkan.PrimaryDevice, GlobalBufferSize, VK_SHARING_MODE_EXCLUSIVE,
                VK_MEMORY_CPU_TO_GPU_PREFERRED,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                &Ground->BufferGlobals))
    {
        Assert(0);
    }
    
    // Chunks
    Ground->MaxChunks = 10;
    VkDeviceSize ChunkBufferSize = VH_PaddedStorageBuffer(Ground->MaxChunks * sizeof(chunk_data));
    ChunkBufferSize = ChunkBufferSize < 256 ? 256 : ChunkBufferSize;
    if (VH_CreateBuffer(GlobalVulkan.PrimaryGPU, GlobalVulkan.PrimaryDevice, ChunkBufferSize, VK_SHARING_MODE_EXCLUSIVE,
                VK_MEMORY_CPU_TO_GPU_PREFERRED,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                &Ground->BufferChunks))
    {
        Assert(0);
    }

    // ALLOCATE POOL Globals
    VH_AllocateDescriptor(&Ground->SetLayoutGlobals, Ground->DescriptorPool, &Ground->DescriptorGlobals);

    VkDescriptorBufferInfo BufferInfoViewPort;
    BufferInfoViewPort.buffer = Ground->BufferGlobals.Buffer; // VkBuffer   buffer;
    BufferInfoViewPort.offset = 0; //FrameIndex * PaddedUniformBuffer(sizeof(GPUSimulationData)); // VkDeviceSize   offset;
    BufferInfoViewPort.range  = sizeof(v4); // VkDeviceSize   range;

    VkWriteDescriptorSet WriteSetViewPort =
        VH_WriteDescriptor(0,Ground->DescriptorGlobals, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, &BufferInfoViewPort);

    VkDescriptorBufferInfo BufferInfoLOD;
    BufferInfoLOD.buffer = Ground->BufferGlobals.Buffer; // VkBuffer   buffer;
    BufferInfoLOD.offset = 0;//PaddedUniformBuffer(BufferInfoViewPort.range);
    BufferInfoLOD.range  = sizeof(ground_voxel_LOD); // VkDeviceSize   range;

    VkWriteDescriptorSet WriteSetLOD =
        VH_WriteDescriptor(1,Ground->DescriptorGlobals, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, &BufferInfoLOD);

    // ALLOCATE POOL Chunks
    VH_AllocateDescriptor(&Ground->SetLayoutChunks, Ground->DescriptorPool, &Ground->DescriptorChunks);

    VkDescriptorBufferInfo BufferChunks;
    BufferChunks.buffer = Ground->BufferChunks.Buffer; // VkBuffer   buffer;
    BufferChunks.offset = 0; //FrameIndex * PaddedUniformBuffer(sizeof(GPUSimulationData)); // VkDeviceSize   offset;
    BufferChunks.range  = Ground->BufferChunks.MemoryRequirements.size; // VkDeviceSize   range;

    VkWriteDescriptorSet WriteSetChunks =
        VH_WriteDescriptor(0,Ground->DescriptorChunks, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &BufferChunks);

    VkWriteDescriptorSet WriteSets[] = 
    {
        WriteSetViewPort,
        WriteSetLOD,
        WriteSetChunks
    };

    vkUpdateDescriptorSets(GlobalVulkan.PrimaryDevice, ArrayCount(WriteSets), WriteSets, 0, nullptr);

    // COMMAND BUFFERS
    
    if (VH_CreateCommandPool(
                GlobalVulkan.PrimaryDevice,
                GlobalVulkan.PresentationQueueFamilyIndex,
                VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                &Ground->CommandPool 
                )) return 1;

    if (VH_CreateCommandBuffers(
                GlobalVulkan.PrimaryDevice,
                Ground->CommandPool,
                ArrayCount(Ground->CommandBuffer),
                &Ground->CommandBuffer[0])) return 1;

    // Fill the buffer with 2 triangles mesh
    void * WriteToAddr;

    // 2 triangles [-1,+1]
    ground_vertex_shader_input Data[6] = {

        {V3(-1.0f,-1.0f,0.0f),V2(0,0)},
        {V3(-1.0f, 1.0f,0.0f),V2(0,1)},
        {V3( 1.0f, 1.0f,0.0f),V2(1,1)},

        {V3( 1.0f, 1.0f,0.0f),V2(1,1)},
        {V3( 1.0f,-1.0f,0.0f),V2(1,0)},
        {V3(-1.0f,-1.0f,0.0f),V2(0,0)}
    };

    u32 SizeVertexBuffer = sizeof(Data);
    if (VH_CreateBuffer(
                GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice, 
                SizeVertexBuffer, VK_SHARING_MODE_EXCLUSIVE,VK_MEMORY_GPU,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                &Ground->VertexBuffer))
    {
        return 1;
    }

    VK_CHECK(vkMapMemory(GlobalVulkan.PrimaryDevice,
                         GlobalVulkan.TransferBitBuffer.DeviceMemory, 0 , SizeVertexBuffer, 0, &WriteToAddr));

    memcpy(WriteToAddr, Data, SizeVertexBuffer);

    vkUnmapMemory(GlobalVulkan.PrimaryDevice,GlobalVulkan.TransferBitBuffer.DeviceMemory);

    if (VH_CopyBuffer(GlobalVulkan.TransferBitCommandBuffer, 
                     GlobalVulkan.TransferBitBuffer.Buffer,Ground->VertexBuffer.Buffer, SizeVertexBuffer, 0))
    {
        Log("Failed to copy data from buffer to gpu\n");
        return 1;
    }

    VkShaderModuleCreateInfo ShaderModuleCreateInfo;

    ShaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO; // VkStructureType sType;
    ShaderModuleCreateInfo.pNext    = 0;                                           // Void * pNext;
    ShaderModuleCreateInfo.flags    = 0;                                           // VkShaderModuleCreateFlags flags;
    ShaderModuleCreateInfo.codeSize = Files[0].Size;                                        // size_t codeSize;
    ShaderModuleCreateInfo.pCode    = (u32 *)Files[0].Base;                            // Typedef * pCode;

    if (VK_FAILS(vkCreateShaderModule(GlobalVulkan.PrimaryDevice, &ShaderModuleCreateInfo, 0, &Ground->ShaderVertexBuildDensity)))
    {
        Assert(0);
    }

    VkShaderModuleCreateInfo ShaderModuleCreateInfo2;

    ShaderModuleCreateInfo2.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO; // VkStructureType sType;
    ShaderModuleCreateInfo2.pNext    = 0;                                           // Void * pNext;
    ShaderModuleCreateInfo2.flags    = 0;                                           // VkShaderModuleCreateFlags flags;
    ShaderModuleCreateInfo2.codeSize = Files[1].Size;                                        // size_t codeSize;
    ShaderModuleCreateInfo2.pCode    = (u32 *)Files[1].Base;                            // Typedef * pCode;

    if (VK_FAILS(vkCreateShaderModule(GlobalVulkan.PrimaryDevice, &ShaderModuleCreateInfo2, 0, &Ground->ShaderGeometryBuildDensity)))
    {
        Assert(0);
    }

    VkShaderModuleCreateInfo ShaderModuleCreateInfo3;

    ShaderModuleCreateInfo3.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO; // VkStructureType sType;
    ShaderModuleCreateInfo3.pNext    = 0;                                           // Void * pNext;
    ShaderModuleCreateInfo3.flags    = 0;                                           // VkShaderModuleCreateFlags flags;
    ShaderModuleCreateInfo3.codeSize = Files[2].Size;                                        // size_t codeSize;
    ShaderModuleCreateInfo3.pCode    = (u32 *)Files[2].Base;                            // Typedef * pCode;

    if (VK_FAILS(vkCreateShaderModule(GlobalVulkan.PrimaryDevice, &ShaderModuleCreateInfo3, 0, &Ground->ShaderFragmentBuildDensity)))
    {
        Assert(0);
    }

    GroundCreateDensityVolumePass(GlobalVulkan.PrimaryGPU,GlobalVulkan.PrimaryDevice);

    GroundCreatePipelineBuildDensities();

    return 0;
}


i32
GroundRenderPasses(m4 * MVP)
{
    ground_info * Ground = &GlobalVulkan.GroundInfo;

    WaitForRender();

#if 0
    if (VulkanSetCurrentImageSwap())
    {
        Log("Couldn't fetch current swapchain Image index\n");
        return 1;
    }
#endif

    //VkCommandBuffer cmd = Ground->CommandBuffer[0];
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;
    VK_CHECK(vkResetCommandBuffer(cmd, 0));


    VkCommandBufferBeginInfo CommandBufferBeginInfo;
    CommandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; // VkStructureType   sType;
    CommandBufferBeginInfo.pNext            = 0;                                           // Void * pNext;
    CommandBufferBeginInfo.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // VkCommandBufferUsageFlags   flags;
    CommandBufferBeginInfo.pInheritanceInfo = 0;                                           // Typedef * pInheritanceInfo;

    VK_CHECK(vkBeginCommandBuffer(cmd, &CommandBufferBeginInfo));

    v4 ClearColor = V4(0,0,0,0);
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
#if 0
    RenderPassBeginInfo.renderPass        = GlobalVulkan.RenderPass;                        // VkRenderPass renderPass;
    RenderPassBeginInfo.framebuffer       = GlobalVulkan.Framebuffers[GlobalVulkan.CurrentSwapchainImageIndex]; // VkFramebuffer framebuffer;
    RenderPassBeginInfo.renderArea.extent = GlobalVulkan.WindowExtension;                   // VkRect2D renderArea;
#else
    RenderPassBeginInfo.renderPass        = Ground->DensityVolumePass.Renderpass;                        // VkRenderPass renderPass;
    RenderPassBeginInfo.framebuffer       = Ground->DensityVolumePass.Framebuffer; // VkFramebuffer framebuffer;
    RenderPassBeginInfo.renderArea.extent.width = 33;
    RenderPassBeginInfo.renderArea.extent.height = 33;
#endif
    RenderPassBeginInfo.clearValueCount   = ArrayCount(ClearAttachments);                   // u32_t clearValueCount;
    RenderPassBeginInfo.pClearValues      = &ClearAttachments[0];                           // Typedef * pClearValues;

    vkCmdBeginRenderPass(cmd, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Ground->PipelineBuildDensities.Pipeline);

    u32 UniformOffset[] = {
        0,(u32)VH_PaddedUniformBuffer(sizeof(v4))
        //PaddedUniformBuffer(sizeof(ground_voxel_LOD))
    };

    vkCmdBindDescriptorSets(cmd, 
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            Ground->PipelineBuildDensities.PipelineLayout, 
                            0, 1, 
                            &Ground->DescriptorGlobals,
                            2, (u32 *)&UniformOffset[0]);

    vkCmdBindDescriptorSets(cmd, 
                            VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            Ground->PipelineBuildDensities.PipelineLayout, 
                            1, 1, 
                            &Ground->DescriptorChunks,
                            0, nullptr);

    VkDeviceSize OffsetVertex = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &Ground->VertexBuffer.Buffer, &OffsetVertex);

    void * WriteToAddr;

    v4 ViewPortData = V4(8.0f,6.0f,1.0f/8.0f,1.0f/6.0f);

    ground_voxel_LOD VoxelLODData;
    
    VoxelLODData.VoxelDim                       = 33;                        // n*4 + 1. This is the number of CORNERS in a block.
                                                                             // For 3 levels of details you want: (corners - 1) to be multiple of 4
    VoxelLODData.VoxelDimMinusOne               = VoxelLODData.VoxelDim - 1; // This is the number of CELLS in a block
    VoxelLODData.wsVoxelSize                    = V2(1.0f/31.0f,0);          // could be 1/63, 1/31, 1/15 depending on LOD
    VoxelLODData.wsChunkSize                    = V2(1.0f,0); // This drives the LOD 1, 2 or 4
    VoxelLODData.InvVoxelDim                    = V2(1.0f/(r32)VoxelLODData.VoxelDim, 0);
    VoxelLODData.InvVoxelDimMinusOne            = V2(1.0f/(r32)VoxelLODData.VoxelDimMinusOne, 0);
    VoxelLODData.Margin                         = 4;
    VoxelLODData.VoxelDimPlusMargins            = VoxelLODData.VoxelDim + 2*VoxelLODData.Margin;
    VoxelLODData.VoxelDimPlusMarginsMinusOne    = VoxelLODData.VoxelDimPlusMargins - 1;
    VoxelLODData._Padding01                     = 0.0f;
    VoxelLODData.InvVoxelDimPlusMargins         = V2(1.0f/(r32)VoxelLODData.VoxelDimPlusMargins, 0);
    VoxelLODData.InvVoxelDimPlusMarginsMinusOne = V2(1.0f/(r32)VoxelLODData.VoxelDimPlusMarginsMinusOne, 0);

    u32 GlobalBufferTotalSize = (u32)VH_PaddedUniformBuffer(sizeof(v4)) + (u32)VH_PaddedUniformBuffer(sizeof(ground_voxel_LOD));

    VK_CHECK(vkMapMemory(GlobalVulkan.PrimaryDevice,
                         Ground->BufferGlobals.DeviceMemory, 
                         0, GlobalBufferTotalSize, 0, &WriteToAddr));

    memcpy(WriteToAddr, (void *)&ViewPortData, sizeof(v4));

    WriteToAddr = (void *)((u8 *)WriteToAddr + VH_PaddedUniformBuffer(sizeof(v4)));
    memcpy(WriteToAddr, (void *)&VoxelLODData, sizeof(ground_voxel_LOD));

    vkUnmapMemory(GlobalVulkan.PrimaryDevice,Ground->BufferGlobals.DeviceMemory);

    
    vkCmdPushConstants(cmd,GlobalVulkan.GroundInfo.PipelineLayoutBuildDensities,VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(m4),MVP);
    u32 CountChunksDrawn = 0;

    VK_CHECK(vkMapMemory(GlobalVulkan.PrimaryDevice,
                         Ground->BufferChunks.DeviceMemory, 
                         0, Ground->BufferChunks.MemoryRequirements.size, 0, &WriteToAddr));

    chunk_data * ChunkWrite = (chunk_data *)WriteToAddr;

    for (i32 ChunkX = 0; ChunkX < 3; ++ChunkX)
    {
        for (i32 ChunkY = 0; ChunkY < 1; ++ChunkY)
        {
            for (i32 ChunkZ = 0; ChunkZ < 1; ++ChunkZ)
            {
                Assert(CountChunksDrawn <= Ground->MaxChunks);
                v3 ChunkP = V3((r32)ChunkX,(r32)ChunkY,(r32)ChunkZ);
                (ChunkWrite + CountChunksDrawn)->wsChunkPos = ChunkP;
                (ChunkWrite + CountChunksDrawn)->opacity = 0.0f;
                ++CountChunksDrawn;
            }
        }
    }

    vkUnmapMemory(GlobalVulkan.PrimaryDevice,Ground->BufferChunks.DeviceMemory);


    for (u32 DrawCallIndex = 0; DrawCallIndex < CountChunksDrawn; ++DrawCallIndex)
    {
        vkCmdDraw(cmd, 6, (u32)VoxelLODData.VoxelDim, 0 , DrawCallIndex);
        //vkCmdDraw(cmd, 6, 1, 0 , 0);
    }

    //RenderEndPassNoPresent();
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


#if 0
    GroundRenderPassBuildDensities();
    GroundRenderPassListNonEmptyCells();
    GroundRenderPassListVerticesToGenerate();
    GroundRenderPassGenerateVertices();
    GroundRenderPassSplatVertexIDs();
    GroundRenderPassGenerateIndices();
#endif

    return 0;
}


