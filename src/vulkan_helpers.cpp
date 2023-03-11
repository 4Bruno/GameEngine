#include "vulkan_local.h"

VkDescriptorSetLayoutBinding
VH_CreateDescriptorSetLayoutBinding(u32 BindingSlot,VkDescriptorType DescriptorType,VkShaderStageFlags ShaderStageFlags)
{
    VkDescriptorSetLayoutBinding Result;
    Result.binding            = BindingSlot; // uint32_t   binding;
    Result.descriptorType     = DescriptorType; // VkDescriptorType   descriptorType;
    Result.descriptorCount    = 1; // uint32_t   descriptorCount;
    Result.stageFlags         = ShaderStageFlags; // VkShaderStageFlags   stageFlags;
    Result.pImmutableSamplers = 0; // Typedef * pImmutableSamplers;

    return Result;
}

//https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanTools.cpp
b32
VH_GetSupportedDepthFormat(VkPhysicalDevice PhysicalDevice, VkFormat *DepthFormat, b32 UseHighestPrecision)
{
    b32 DepthFormatSupported = false;

    // Since all depth formats may be optional, we need to find a suitable depth format to use
    // Start with the highest precision packed format
    VkFormat DepthFormats[] = 
    {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };

    for (u32 FormatIndex = 0;
             FormatIndex < ArrayCount(DepthFormats);
             ++FormatIndex)
    {
        VkFormatProperties formatProps;
        VkFormat Format = DepthFormats[FormatIndex];
        vkGetPhysicalDeviceFormatProperties(PhysicalDevice, Format, &formatProps);

        // Format must support depth stencil attachment for optimal tiling
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            DepthFormatSupported = true;
            *DepthFormat = Format;
            if (UseHighestPrecision) break;
        }
    }

    return DepthFormatSupported;
}

void
PrintFormatsSupportedByColorAttachment()
{
    VkFormat FormatsToCheck[] = 
    {
        VK_FORMAT_UNDEFINED ,
        VK_FORMAT_R4G4_UNORM_PACK8 ,
        VK_FORMAT_R4G4B4A4_UNORM_PACK16 ,
        VK_FORMAT_B4G4R4A4_UNORM_PACK16 ,
        VK_FORMAT_R5G6B5_UNORM_PACK16 ,
        VK_FORMAT_B5G6R5_UNORM_PACK16 ,
        VK_FORMAT_R5G5B5A1_UNORM_PACK16 ,
        VK_FORMAT_B5G5R5A1_UNORM_PACK16 ,
        VK_FORMAT_A1R5G5B5_UNORM_PACK16 ,
        VK_FORMAT_R8_UNORM ,
        VK_FORMAT_R8_SNORM ,
        VK_FORMAT_R8_USCALED ,
        VK_FORMAT_R8_SSCALED ,
        VK_FORMAT_R8_UINT ,
        VK_FORMAT_R8_SINT ,
        VK_FORMAT_R8_SRGB ,
        VK_FORMAT_R8G8_UNORM ,
        VK_FORMAT_R8G8_SNORM ,
        VK_FORMAT_R8G8_USCALED ,
        VK_FORMAT_R8G8_SSCALED ,
        VK_FORMAT_R8G8_UINT ,
        VK_FORMAT_R8G8_SINT ,
        VK_FORMAT_R8G8_SRGB ,
        VK_FORMAT_R8G8B8_UNORM ,
        VK_FORMAT_R8G8B8_SNORM ,
        VK_FORMAT_R8G8B8_USCALED ,
        VK_FORMAT_R8G8B8_SSCALED ,
        VK_FORMAT_R8G8B8_UINT ,
        VK_FORMAT_R8G8B8_SINT ,
        VK_FORMAT_R8G8B8_SRGB ,
        VK_FORMAT_B8G8R8_UNORM ,
        VK_FORMAT_B8G8R8_SNORM ,
        VK_FORMAT_B8G8R8_USCALED ,
        VK_FORMAT_B8G8R8_SSCALED ,
        VK_FORMAT_B8G8R8_UINT ,
        VK_FORMAT_B8G8R8_SINT ,
        VK_FORMAT_B8G8R8_SRGB ,
        VK_FORMAT_R8G8B8A8_UNORM ,
        VK_FORMAT_R8G8B8A8_SNORM ,
        VK_FORMAT_R8G8B8A8_USCALED ,
        VK_FORMAT_R8G8B8A8_SSCALED ,
        VK_FORMAT_R8G8B8A8_UINT ,
        VK_FORMAT_R8G8B8A8_SINT ,
        VK_FORMAT_R8G8B8A8_SRGB ,
        VK_FORMAT_B8G8R8A8_UNORM ,
        VK_FORMAT_B8G8R8A8_SNORM ,
        VK_FORMAT_B8G8R8A8_USCALED ,
        VK_FORMAT_B8G8R8A8_SSCALED ,
        VK_FORMAT_B8G8R8A8_UINT ,
        VK_FORMAT_B8G8R8A8_SINT ,
        VK_FORMAT_B8G8R8A8_SRGB ,
        VK_FORMAT_A8B8G8R8_UNORM_PACK32 ,
        VK_FORMAT_A8B8G8R8_SNORM_PACK32 ,
        VK_FORMAT_A8B8G8R8_USCALED_PACK32 ,
        VK_FORMAT_A8B8G8R8_SSCALED_PACK32 ,
        VK_FORMAT_A8B8G8R8_UINT_PACK32 ,
        VK_FORMAT_A8B8G8R8_SINT_PACK32 ,
        VK_FORMAT_A8B8G8R8_SRGB_PACK32 ,
        VK_FORMAT_A2R10G10B10_UNORM_PACK32 ,
        VK_FORMAT_A2R10G10B10_SNORM_PACK32 ,
        VK_FORMAT_A2R10G10B10_USCALED_PACK32 ,
        VK_FORMAT_A2R10G10B10_SSCALED_PACK32 ,
        VK_FORMAT_A2R10G10B10_UINT_PACK32 ,
        VK_FORMAT_A2R10G10B10_SINT_PACK32 ,
        VK_FORMAT_A2B10G10R10_UNORM_PACK32 ,
        VK_FORMAT_A2B10G10R10_SNORM_PACK32 ,
        VK_FORMAT_A2B10G10R10_USCALED_PACK32 ,
        VK_FORMAT_A2B10G10R10_SSCALED_PACK32 ,
        VK_FORMAT_A2B10G10R10_UINT_PACK32 ,
        VK_FORMAT_A2B10G10R10_SINT_PACK32 ,
        VK_FORMAT_R16_UNORM ,
        VK_FORMAT_R16_SNORM ,
        VK_FORMAT_R16_USCALED ,
        VK_FORMAT_R16_SSCALED ,
        VK_FORMAT_R16_UINT ,
        VK_FORMAT_R16_SINT ,
        VK_FORMAT_R16_SFLOAT ,
        VK_FORMAT_R16G16_UNORM ,
        VK_FORMAT_R16G16_SNORM ,
        VK_FORMAT_R16G16_USCALED ,
        VK_FORMAT_R16G16_SSCALED ,
        VK_FORMAT_R16G16_UINT ,
        VK_FORMAT_R16G16_SINT ,
        VK_FORMAT_R16G16_SFLOAT ,
        VK_FORMAT_R16G16B16_UNORM ,
        VK_FORMAT_R16G16B16_SNORM ,
        VK_FORMAT_R16G16B16_USCALED ,
        VK_FORMAT_R16G16B16_SSCALED ,
        VK_FORMAT_R16G16B16_UINT ,
        VK_FORMAT_R16G16B16_SINT ,
        VK_FORMAT_R16G16B16_SFLOAT ,
        VK_FORMAT_R16G16B16A16_UNORM ,
        VK_FORMAT_R16G16B16A16_SNORM ,
        VK_FORMAT_R16G16B16A16_USCALED ,
        VK_FORMAT_R16G16B16A16_SSCALED ,
        VK_FORMAT_R16G16B16A16_UINT ,
        VK_FORMAT_R16G16B16A16_SINT ,
        VK_FORMAT_R16G16B16A16_SFLOAT ,
        VK_FORMAT_R32_UINT ,
        VK_FORMAT_R32_SINT ,
        VK_FORMAT_R32_SFLOAT ,
        VK_FORMAT_R32G32_UINT ,
        VK_FORMAT_R32G32_SINT ,
        VK_FORMAT_R32G32_SFLOAT ,
        VK_FORMAT_R32G32B32_UINT ,
        VK_FORMAT_R32G32B32_SINT ,
        VK_FORMAT_R32G32B32_SFLOAT ,
        VK_FORMAT_R32G32B32A32_UINT ,
        VK_FORMAT_R32G32B32A32_SINT ,
        VK_FORMAT_R32G32B32A32_SFLOAT ,
        VK_FORMAT_R64_UINT ,
        VK_FORMAT_R64_SINT ,
        VK_FORMAT_R64_SFLOAT ,
        VK_FORMAT_R64G64_UINT ,
        VK_FORMAT_R64G64_SINT ,
        VK_FORMAT_R64G64_SFLOAT ,
        VK_FORMAT_R64G64B64_UINT ,
        VK_FORMAT_R64G64B64_SINT ,
        VK_FORMAT_R64G64B64_SFLOAT ,
        VK_FORMAT_R64G64B64A64_UINT ,
        VK_FORMAT_R64G64B64A64_SINT ,
        VK_FORMAT_R64G64B64A64_SFLOAT ,
        VK_FORMAT_B10G11R11_UFLOAT_PACK32 ,
        VK_FORMAT_E5B9G9R9_UFLOAT_PACK32 ,
        VK_FORMAT_D16_UNORM ,
        VK_FORMAT_X8_D24_UNORM_PACK32 ,
        VK_FORMAT_D32_SFLOAT ,
        VK_FORMAT_S8_UINT ,
        VK_FORMAT_D16_UNORM_S8_UINT ,
        VK_FORMAT_D24_UNORM_S8_UINT ,
        VK_FORMAT_D32_SFLOAT_S8_UINT ,
        VK_FORMAT_BC1_RGB_UNORM_BLOCK ,
        VK_FORMAT_BC1_RGB_SRGB_BLOCK ,
        VK_FORMAT_BC1_RGBA_UNORM_BLOCK ,
        VK_FORMAT_BC1_RGBA_SRGB_BLOCK ,
        VK_FORMAT_BC2_UNORM_BLOCK ,
        VK_FORMAT_BC2_SRGB_BLOCK ,
        VK_FORMAT_BC3_UNORM_BLOCK ,
        VK_FORMAT_BC3_SRGB_BLOCK ,
        VK_FORMAT_BC4_UNORM_BLOCK ,
        VK_FORMAT_BC4_SNORM_BLOCK ,
        VK_FORMAT_BC5_UNORM_BLOCK ,
        VK_FORMAT_BC5_SNORM_BLOCK ,
        VK_FORMAT_BC6H_UFLOAT_BLOCK ,
        VK_FORMAT_BC6H_SFLOAT_BLOCK ,
        VK_FORMAT_BC7_UNORM_BLOCK ,
        VK_FORMAT_BC7_SRGB_BLOCK ,
        VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK ,
        VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK ,
        VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK ,
        VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK ,
        VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK ,
        VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK ,
        VK_FORMAT_EAC_R11_UNORM_BLOCK ,
        VK_FORMAT_EAC_R11_SNORM_BLOCK ,
        VK_FORMAT_EAC_R11G11_UNORM_BLOCK ,
        VK_FORMAT_EAC_R11G11_SNORM_BLOCK ,
        VK_FORMAT_ASTC_4x4_UNORM_BLOCK ,
        VK_FORMAT_ASTC_4x4_SRGB_BLOCK ,
        VK_FORMAT_ASTC_5x4_UNORM_BLOCK ,
        VK_FORMAT_ASTC_5x4_SRGB_BLOCK ,
        VK_FORMAT_ASTC_5x5_UNORM_BLOCK ,
        VK_FORMAT_ASTC_5x5_SRGB_BLOCK ,
        VK_FORMAT_ASTC_6x5_UNORM_BLOCK ,
        VK_FORMAT_ASTC_6x5_SRGB_BLOCK ,
        VK_FORMAT_ASTC_6x6_UNORM_BLOCK ,
        VK_FORMAT_ASTC_6x6_SRGB_BLOCK ,
        VK_FORMAT_ASTC_8x5_UNORM_BLOCK ,
        VK_FORMAT_ASTC_8x5_SRGB_BLOCK ,
        VK_FORMAT_ASTC_8x6_UNORM_BLOCK ,
        VK_FORMAT_ASTC_8x6_SRGB_BLOCK ,
        VK_FORMAT_ASTC_8x8_UNORM_BLOCK ,
        VK_FORMAT_ASTC_8x8_SRGB_BLOCK ,
        VK_FORMAT_ASTC_10x5_UNORM_BLOCK ,
        VK_FORMAT_ASTC_10x5_SRGB_BLOCK ,
        VK_FORMAT_ASTC_10x6_UNORM_BLOCK ,
        VK_FORMAT_ASTC_10x6_SRGB_BLOCK ,
        VK_FORMAT_ASTC_10x8_UNORM_BLOCK ,
        VK_FORMAT_ASTC_10x8_SRGB_BLOCK ,
        VK_FORMAT_ASTC_10x10_UNORM_BLOCK ,
        VK_FORMAT_ASTC_10x10_SRGB_BLOCK ,
        VK_FORMAT_ASTC_12x10_UNORM_BLOCK ,
        VK_FORMAT_ASTC_12x10_SRGB_BLOCK ,
        VK_FORMAT_ASTC_12x12_UNORM_BLOCK ,
        VK_FORMAT_ASTC_12x12_SRGB_BLOCK ,
        VK_FORMAT_G8B8G8R8_422_UNORM ,
        VK_FORMAT_B8G8R8G8_422_UNORM ,
        VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM ,
        VK_FORMAT_G8_B8R8_2PLANE_420_UNORM ,
        VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM ,
        VK_FORMAT_G8_B8R8_2PLANE_422_UNORM ,
        VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM ,
        VK_FORMAT_R10X6_UNORM_PACK16 ,
        VK_FORMAT_R10X6G10X6_UNORM_2PACK16 ,
        VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 ,
        VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 ,
        VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 ,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 ,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 ,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 ,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 ,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 ,
        VK_FORMAT_R12X4_UNORM_PACK16 ,
        VK_FORMAT_R12X4G12X4_UNORM_2PACK16 ,
        VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16 ,
        VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 ,
        VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 ,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 ,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 ,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 ,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 ,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 ,
        VK_FORMAT_G16B16G16R16_422_UNORM ,
        VK_FORMAT_B16G16R16G16_422_UNORM ,
        VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM ,
        VK_FORMAT_G16_B16R16_2PLANE_420_UNORM ,
        VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM ,
        VK_FORMAT_G16_B16R16_2PLANE_422_UNORM ,
        VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM ,
        VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT ,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT ,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT ,
        VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT ,
        VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT ,
        VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT ,
        VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG ,
        VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG ,
        VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG ,
        VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG ,
        VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG ,
        VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG ,
        VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG ,
        VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG ,
        VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT ,
        VK_FORMAT_G8B8G8R8_422_UNORM_KHR ,
        VK_FORMAT_B8G8R8G8_422_UNORM_KHR ,
        VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR ,
        VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR ,
        VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR ,
        VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR ,
        VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR ,
        VK_FORMAT_R10X6_UNORM_PACK16_KHR ,
        VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR ,
        VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR ,
        VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR ,
        VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR ,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR ,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR ,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR ,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR ,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR ,
        VK_FORMAT_R12X4_UNORM_PACK16_KHR ,
        VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR ,
        VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR ,
        VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR ,
        VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR ,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR ,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR ,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR ,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR ,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR ,
        VK_FORMAT_G16B16G16R16_422_UNORM_KHR ,
        VK_FORMAT_B16G16R16G16_422_UNORM_KHR ,
        VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR ,
        VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR ,
        VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR ,
        VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR ,
        VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR ,
        VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT ,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT ,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT ,
        VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT ,
        VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT ,
        VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT ,
    };

    const char * FormatName[] =
    {
        "VK_FORMAT_UNDEFINED" ,
        "VK_FORMAT_R4G4_UNORM_PACK8" ,
        "VK_FORMAT_R4G4B4A4_UNORM_PACK16" ,
        "VK_FORMAT_B4G4R4A4_UNORM_PACK16" ,
        "VK_FORMAT_R5G6B5_UNORM_PACK16" ,
        "VK_FORMAT_B5G6R5_UNORM_PACK16" ,
        "VK_FORMAT_R5G5B5A1_UNORM_PACK16" ,
        "VK_FORMAT_B5G5R5A1_UNORM_PACK16" ,
        "VK_FORMAT_A1R5G5B5_UNORM_PACK16" ,
        "VK_FORMAT_R8_UNORM" ,
        "VK_FORMAT_R8_SNORM" ,
        "VK_FORMAT_R8_USCALED" ,
        "VK_FORMAT_R8_SSCALED" ,
        "VK_FORMAT_R8_UINT" ,
        "VK_FORMAT_R8_SINT" ,
        "VK_FORMAT_R8_SRGB" ,
        "VK_FORMAT_R8G8_UNORM" ,
        "VK_FORMAT_R8G8_SNORM" ,
        "VK_FORMAT_R8G8_USCALED" ,
        "VK_FORMAT_R8G8_SSCALED" ,
        "VK_FORMAT_R8G8_UINT" ,
        "VK_FORMAT_R8G8_SINT" ,
        "VK_FORMAT_R8G8_SRGB" ,
        "VK_FORMAT_R8G8B8_UNORM" ,
        "VK_FORMAT_R8G8B8_SNORM" ,
        "VK_FORMAT_R8G8B8_USCALED" ,
        "VK_FORMAT_R8G8B8_SSCALED" ,
        "VK_FORMAT_R8G8B8_UINT" ,
        "VK_FORMAT_R8G8B8_SINT" ,
        "VK_FORMAT_R8G8B8_SRGB" ,
        "VK_FORMAT_B8G8R8_UNORM" ,
        "VK_FORMAT_B8G8R8_SNORM" ,
        "VK_FORMAT_B8G8R8_USCALED" ,
        "VK_FORMAT_B8G8R8_SSCALED" ,
        "VK_FORMAT_B8G8R8_UINT" ,
        "VK_FORMAT_B8G8R8_SINT" ,
        "VK_FORMAT_B8G8R8_SRGB" ,
        "VK_FORMAT_R8G8B8A8_UNORM" ,
        "VK_FORMAT_R8G8B8A8_SNORM" ,
        "VK_FORMAT_R8G8B8A8_USCALED" ,
        "VK_FORMAT_R8G8B8A8_SSCALED" ,
        "VK_FORMAT_R8G8B8A8_UINT" ,
        "VK_FORMAT_R8G8B8A8_SINT" ,
        "VK_FORMAT_R8G8B8A8_SRGB" ,
        "VK_FORMAT_B8G8R8A8_UNORM" ,
        "VK_FORMAT_B8G8R8A8_SNORM" ,
        "VK_FORMAT_B8G8R8A8_USCALED" ,
        "VK_FORMAT_B8G8R8A8_SSCALED" ,
        "VK_FORMAT_B8G8R8A8_UINT" ,
        "VK_FORMAT_B8G8R8A8_SINT" ,
        "VK_FORMAT_B8G8R8A8_SRGB" ,
        "VK_FORMAT_A8B8G8R8_UNORM_PACK32" ,
        "VK_FORMAT_A8B8G8R8_SNORM_PACK32" ,
        "VK_FORMAT_A8B8G8R8_USCALED_PACK32" ,
        "VK_FORMAT_A8B8G8R8_SSCALED_PACK32" ,
        "VK_FORMAT_A8B8G8R8_UINT_PACK32" ,
        "VK_FORMAT_A8B8G8R8_SINT_PACK32" ,
        "VK_FORMAT_A8B8G8R8_SRGB_PACK32" ,
        "VK_FORMAT_A2R10G10B10_UNORM_PACK32" ,
        "VK_FORMAT_A2R10G10B10_SNORM_PACK32" ,
        "VK_FORMAT_A2R10G10B10_USCALED_PACK32" ,
        "VK_FORMAT_A2R10G10B10_SSCALED_PACK32" ,
        "VK_FORMAT_A2R10G10B10_UINT_PACK32" ,
        "VK_FORMAT_A2R10G10B10_SINT_PACK32" ,
        "VK_FORMAT_A2B10G10R10_UNORM_PACK32" ,
        "VK_FORMAT_A2B10G10R10_SNORM_PACK32" ,
        "VK_FORMAT_A2B10G10R10_USCALED_PACK32" ,
        "VK_FORMAT_A2B10G10R10_SSCALED_PACK32" ,
        "VK_FORMAT_A2B10G10R10_UINT_PACK32" ,
        "VK_FORMAT_A2B10G10R10_SINT_PACK32" ,
        "VK_FORMAT_R16_UNORM" ,
        "VK_FORMAT_R16_SNORM" ,
        "VK_FORMAT_R16_USCALED" ,
        "VK_FORMAT_R16_SSCALED" ,
        "VK_FORMAT_R16_UINT" ,
        "VK_FORMAT_R16_SINT" ,
        "VK_FORMAT_R16_SFLOAT" ,
        "VK_FORMAT_R16G16_UNORM" ,
        "VK_FORMAT_R16G16_SNORM" ,
        "VK_FORMAT_R16G16_USCALED" ,
        "VK_FORMAT_R16G16_SSCALED" ,
        "VK_FORMAT_R16G16_UINT" ,
        "VK_FORMAT_R16G16_SINT" ,
        "VK_FORMAT_R16G16_SFLOAT" ,
        "VK_FORMAT_R16G16B16_UNORM" ,
        "VK_FORMAT_R16G16B16_SNORM" ,
        "VK_FORMAT_R16G16B16_USCALED" ,
        "VK_FORMAT_R16G16B16_SSCALED" ,
        "VK_FORMAT_R16G16B16_UINT" ,
        "VK_FORMAT_R16G16B16_SINT" ,
        "VK_FORMAT_R16G16B16_SFLOAT" ,
        "VK_FORMAT_R16G16B16A16_UNORM" ,
        "VK_FORMAT_R16G16B16A16_SNORM" ,
        "VK_FORMAT_R16G16B16A16_USCALED" ,
        "VK_FORMAT_R16G16B16A16_SSCALED" ,
        "VK_FORMAT_R16G16B16A16_UINT" ,
        "VK_FORMAT_R16G16B16A16_SINT" ,
        "VK_FORMAT_R16G16B16A16_SFLOAT" ,
        "VK_FORMAT_R32_UINT" ,
        "VK_FORMAT_R32_SINT" ,
        "VK_FORMAT_R32_SFLOAT" ,
        "VK_FORMAT_R32G32_UINT" ,
        "VK_FORMAT_R32G32_SINT" ,
        "VK_FORMAT_R32G32_SFLOAT" ,
        "VK_FORMAT_R32G32B32_UINT" ,
        "VK_FORMAT_R32G32B32_SINT" ,
        "VK_FORMAT_R32G32B32_SFLOAT" ,
        "VK_FORMAT_R32G32B32A32_UINT" ,
        "VK_FORMAT_R32G32B32A32_SINT" ,
        "VK_FORMAT_R32G32B32A32_SFLOAT" ,
        "VK_FORMAT_R64_UINT" ,
        "VK_FORMAT_R64_SINT" ,
        "VK_FORMAT_R64_SFLOAT" ,
        "VK_FORMAT_R64G64_UINT" ,
        "VK_FORMAT_R64G64_SINT" ,
        "VK_FORMAT_R64G64_SFLOAT" ,
        "VK_FORMAT_R64G64B64_UINT" ,
        "VK_FORMAT_R64G64B64_SINT" ,
        "VK_FORMAT_R64G64B64_SFLOAT" ,
        "VK_FORMAT_R64G64B64A64_UINT" ,
        "VK_FORMAT_R64G64B64A64_SINT" ,
        "VK_FORMAT_R64G64B64A64_SFLOAT" ,
        "VK_FORMAT_B10G11R11_UFLOAT_PACK32" ,
        "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32" ,
        "VK_FORMAT_D16_UNORM" ,
        "VK_FORMAT_X8_D24_UNORM_PACK32" ,
        "VK_FORMAT_D32_SFLOAT" ,
        "VK_FORMAT_S8_UINT" ,
        "VK_FORMAT_D16_UNORM_S8_UINT" ,
        "VK_FORMAT_D24_UNORM_S8_UINT" ,
        "VK_FORMAT_D32_SFLOAT_S8_UINT" ,
        "VK_FORMAT_BC1_RGB_UNORM_BLOCK" ,
        "VK_FORMAT_BC1_RGB_SRGB_BLOCK" ,
        "VK_FORMAT_BC1_RGBA_UNORM_BLOCK" ,
        "VK_FORMAT_BC1_RGBA_SRGB_BLOCK" ,
        "VK_FORMAT_BC2_UNORM_BLOCK" ,
        "VK_FORMAT_BC2_SRGB_BLOCK" ,
        "VK_FORMAT_BC3_UNORM_BLOCK" ,
        "VK_FORMAT_BC3_SRGB_BLOCK" ,
        "VK_FORMAT_BC4_UNORM_BLOCK" ,
        "VK_FORMAT_BC4_SNORM_BLOCK" ,
        "VK_FORMAT_BC5_UNORM_BLOCK" ,
        "VK_FORMAT_BC5_SNORM_BLOCK" ,
        "VK_FORMAT_BC6H_UFLOAT_BLOCK" ,
        "VK_FORMAT_BC6H_SFLOAT_BLOCK" ,
        "VK_FORMAT_BC7_UNORM_BLOCK" ,
        "VK_FORMAT_BC7_SRGB_BLOCK" ,
        "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK" ,
        "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK" ,
        "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK" ,
        "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK" ,
        "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK" ,
        "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK" ,
        "VK_FORMAT_EAC_R11_UNORM_BLOCK" ,
        "VK_FORMAT_EAC_R11_SNORM_BLOCK" ,
        "VK_FORMAT_EAC_R11G11_UNORM_BLOCK" ,
        "VK_FORMAT_EAC_R11G11_SNORM_BLOCK" ,
        "VK_FORMAT_ASTC_4x4_UNORM_BLOCK" ,
        "VK_FORMAT_ASTC_4x4_SRGB_BLOCK" ,
        "VK_FORMAT_ASTC_5x4_UNORM_BLOCK" ,
        "VK_FORMAT_ASTC_5x4_SRGB_BLOCK" ,
        "VK_FORMAT_ASTC_5x5_UNORM_BLOCK" ,
        "VK_FORMAT_ASTC_5x5_SRGB_BLOCK" ,
        "VK_FORMAT_ASTC_6x5_UNORM_BLOCK" ,
        "VK_FORMAT_ASTC_6x5_SRGB_BLOCK" ,
        "VK_FORMAT_ASTC_6x6_UNORM_BLOCK" ,
        "VK_FORMAT_ASTC_6x6_SRGB_BLOCK" ,
        "VK_FORMAT_ASTC_8x5_UNORM_BLOCK" ,
        "VK_FORMAT_ASTC_8x5_SRGB_BLOCK" ,
        "VK_FORMAT_ASTC_8x6_UNORM_BLOCK" ,
        "VK_FORMAT_ASTC_8x6_SRGB_BLOCK" ,
        "VK_FORMAT_ASTC_8x8_UNORM_BLOCK" ,
        "VK_FORMAT_ASTC_8x8_SRGB_BLOCK" ,
        "VK_FORMAT_ASTC_10x5_UNORM_BLOCK" ,
        "VK_FORMAT_ASTC_10x5_SRGB_BLOCK" ,
        "VK_FORMAT_ASTC_10x6_UNORM_BLOCK" ,
        "VK_FORMAT_ASTC_10x6_SRGB_BLOCK" ,
        "VK_FORMAT_ASTC_10x8_UNORM_BLOCK" ,
        "VK_FORMAT_ASTC_10x8_SRGB_BLOCK" ,
        "VK_FORMAT_ASTC_10x10_UNORM_BLOCK" ,
        "VK_FORMAT_ASTC_10x10_SRGB_BLOCK" ,
        "VK_FORMAT_ASTC_12x10_UNORM_BLOCK" ,
        "VK_FORMAT_ASTC_12x10_SRGB_BLOCK" ,
        "VK_FORMAT_ASTC_12x12_UNORM_BLOCK" ,
        "VK_FORMAT_ASTC_12x12_SRGB_BLOCK" ,
        "VK_FORMAT_G8B8G8R8_422_UNORM" ,
        "VK_FORMAT_B8G8R8G8_422_UNORM" ,
        "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM" ,
        "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM" ,
        "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM" ,
        "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM" ,
        "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM" ,
        "VK_FORMAT_R10X6_UNORM_PACK16" ,
        "VK_FORMAT_R10X6G10X6_UNORM_2PACK16" ,
        "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16" ,
        "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16" ,
        "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16" ,
        "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16" ,
        "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16" ,
        "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16" ,
        "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16" ,
        "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16" ,
        "VK_FORMAT_R12X4_UNORM_PACK16" ,
        "VK_FORMAT_R12X4G12X4_UNORM_2PACK16" ,
        "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16" ,
        "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16" ,
        "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16" ,
        "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16" ,
        "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16" ,
        "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16" ,
        "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16" ,
        "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16" ,
        "VK_FORMAT_G16B16G16R16_422_UNORM" ,
        "VK_FORMAT_B16G16R16G16_422_UNORM" ,
        "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM" ,
        "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM" ,
        "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM" ,
        "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM" ,
        "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM" ,
        "VK_FORMAT_G8_B8R8_2PLANE_444_UNORM" ,
        "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16" ,
        "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16" ,
        "VK_FORMAT_G16_B16R16_2PLANE_444_UNORM" ,
        "VK_FORMAT_A4R4G4B4_UNORM_PACK16" ,
        "VK_FORMAT_A4B4G4R4_UNORM_PACK16" ,
        "VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK" ,
        "VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK" ,
        "VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK" ,
        "VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK" ,
        "VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK" ,
        "VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK" ,
        "VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK" ,
        "VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK" ,
        "VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK" ,
        "VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK" ,
        "VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK" ,
        "VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK" ,
        "VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK" ,
        "VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK" ,
        "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG" ,
        "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG" ,
        "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG" ,
        "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG" ,
        "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG" ,
        "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG" ,
        "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG" ,
        "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG" ,
        "VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT" ,
        "VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT" ,
        "VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT" ,
        "VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT" ,
        "VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT" ,
        "VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT" ,
        "VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT" ,
        "VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT" ,
        "VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT" ,
        "VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT" ,
        "VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT" ,
        "VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT" ,
        "VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT" ,
        "VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT" ,
        "VK_FORMAT_G8B8G8R8_422_UNORM_KHR" ,
        "VK_FORMAT_B8G8R8G8_422_UNORM_KHR" ,
        "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR" ,
        "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR" ,
        "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR" ,
        "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR" ,
        "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR" ,
        "VK_FORMAT_R10X6_UNORM_PACK16_KHR" ,
        "VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR" ,
        "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR" ,
        "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR" ,
        "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR" ,
        "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR" ,
        "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR" ,
        "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR" ,
        "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR" ,
        "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR" ,
        "VK_FORMAT_R12X4_UNORM_PACK16_KHR" ,
        "VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR" ,
        "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR" ,
        "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR" ,
        "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR" ,
        "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR" ,
        "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR" ,
        "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR" ,
        "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR" ,
        "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR" ,
        "VK_FORMAT_G16B16G16R16_422_UNORM_KHR" ,
        "VK_FORMAT_B16G16R16G16_422_UNORM_KHR" ,
        "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR" ,
        "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR" ,
        "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR" ,
        "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR" ,
        "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR" ,
        "VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT" ,
        "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT" ,
        "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT" ,
        "VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT" ,
        "VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT" ,
        "VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT" ,
    };
    
    for (u32 FormatIndex = 0;
             FormatIndex < ArrayCount(FormatsToCheck);
             ++FormatIndex)
    {
        VkFormatProperties FormatProp;
        VkFormat Format = FormatsToCheck[FormatIndex];
        vkGetPhysicalDeviceFormatProperties(GlobalVulkan.PrimaryGPU, Format, &FormatProp);

        if (FormatProp.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
        {
            Logn("Format supported %s",FormatName[FormatIndex]);
        }
    };
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


#if 0
i32
VH_CreateBuffer(VkPhysicalDevice PhysicalDevice,VkDevice Device, 
                    VkDeviceSize Size, 
                    VkSharingMode SharingMode,
                    VkMemoryPropertyFlags PropertyFlags, 
                    VkBufferUsageFlags Usage,
                    vulkan_buffer * Buffer,
                    u32 SharedBufferQueueFamilyIndexCount,
                    u32 * SharedBufferQueueFamilyIndexArray)
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

    gpu_arena Arena = VH_AllocateMemory(PhysicalDevice,Device, Buffer->MemoryRequirements,PropertyFlags);


    return 0;
}
#endif

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
VH_PipelineBuilder(vulkan_pipeline * VulkanPipeline,VkDevice Device, VkRenderPass RenderPass, u32 Subpass)
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

VkImageCreateInfo
VH_DepthBufferCreateInfo(VkExtent3D Extent)
{
    VkFormat Format = VK_FORMAT_D32_SFLOAT;
    VkImageUsageFlags Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageCreateInfo ImageCreateInfo = VH_CreateImageCreateInfo2D(Extent, Format, Usage);

    return ImageCreateInfo;
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
                           VkDevice Device, u32 Size,
                           gpu_arena * Arena)
{
    Arena->MemoryIndexType = -1; // MemoryIndexType   MemoryIndexType
    Arena->GPU             = PhysicalDevice; // GPU   GPU
    Arena->Device          = Device; // Device   Device
    Arena->MaxSize         = Size; // MaxSize   MaxSize
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

    return 0;
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

void
VH_FreeMemory()
{
    for (u32 MemoryArenaIndex = 0;
                MemoryArenaIndex < GlobalVulkan.DeviceMemoryPools.Count;
                ++MemoryArenaIndex)
    {
        device_memory_pool * DeviceMemoryPool = GlobalVulkan.DeviceMemoryPools.DeviceMemoryPool + MemoryArenaIndex;
                
        if (DeviceMemoryPool->Size > 0)
        {
            if ( VK_VALID_HANDLE(DeviceMemoryPool->DeviceMemory) )
            {
                vkFreeMemory(DeviceMemoryPool->Device,DeviceMemoryPool->DeviceMemory,0);
            }
            DeviceMemoryPool->DeviceMemory = VK_NULL_HANDLE;
            DeviceMemoryPool->Device       = VK_NULL_HANDLE;
            DeviceMemoryPool->Size         = 0; // Size Size
        }
    }
}
u32 
makeAccessMaskPipelineStageFlags(
        u32 accessMask, 
        VkPipelineStageFlags supportedShaderBits = 
                    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT                   | 
                    VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT     | 
                    VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT  | 
                    VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT                 | 
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT                 | 
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
                    )
{
    const u32 accessPipes[] = {
        VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
        VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
        VK_ACCESS_INDEX_READ_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VK_ACCESS_UNIFORM_READ_BIT,
        supportedShaderBits,
        VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        supportedShaderBits,
        VK_ACCESS_SHADER_WRITE_BIT,
        supportedShaderBits,
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_ACCESS_HOST_READ_BIT,
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_ACCESS_HOST_WRITE_BIT,
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        0,
        VK_ACCESS_MEMORY_WRITE_BIT,
        0,
#if 0//VK_NV_device_generated_commands
        VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV,
        VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV,
        VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV,
        VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV,
#endif
#if 0 
        VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV,
        VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV | supportedShaderBits | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
        VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV,
        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
#endif
    };
    if(!accessMask)
    {
        return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }

    u32 pipes = 0;

    for(u32 i = 0; i < ArrayCount(accessPipes); i += 2)
    {
        if(accessPipes[i] & accessMask)
        {
            pipes |= accessPipes[i + 1];
        }
    }

    Assert(pipes != 0);

    return pipes;
}

VkImageMemoryBarrier 
makeImageMemoryBarrier(VkImage            img,
                        VkAccessFlags      srcAccess,
                        VkAccessFlags      dstAccess,
                        VkImageLayout      oldLayout,
                        VkImageLayout      newLayout,
                        VkImageAspectFlags aspectMask)
{
    VkImageMemoryBarrier barrier        = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.srcAccessMask               = srcAccess;
    barrier.dstAccessMask               = dstAccess;
    barrier.oldLayout                   = oldLayout;
    barrier.newLayout                   = newLayout;
    barrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
    barrier.srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                       = img;
    barrier.subresourceRange            = {0};
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    return barrier;
}

// A simple wrapper for writing a vkCmdPipelineBarrier for doing things such as
// image layout transitions.
inline void 
cmdImageTransition(VkCommandBuffer    cmd,
                               VkImage            img,
                               VkImageAspectFlags aspects,
                               VkAccessFlags      src,
                               VkAccessFlags      dst,
                               VkImageLayout      oldLayout,
                               VkImageLayout      newLayout)
{

  VkPipelineStageFlags srcPipe = makeAccessMaskPipelineStageFlags(src);
  VkPipelineStageFlags dstPipe = makeAccessMaskPipelineStageFlags(dst);
  VkImageMemoryBarrier barrier = 
      makeImageMemoryBarrier(img, src, dst, oldLayout, newLayout, aspects);

  vkCmdPipelineBarrier(cmd, srcPipe, dstPipe, VK_FALSE, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);
}

void
VH_TranstionTo(VkCommandBuffer cmdBuffer, vulkan_image * VulkanImage, VkImageLayout   dstLayout, VkAccessFlags   dstAccesses)
{
    //https://github.com/nvpro-samples/vk_order_independent_transparency/blob/90a4a007187ce7281c2eafb3b447cfb581616d1e/utilities_vk.h
    // Note that in larger applications, we could batch together pipeline
    // barriers for better performance!

    // Maps to barrier.subresourceRange.aspectMask
    VkImageAspectFlags aspectMask = 0;
    if(dstLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if(VulkanImage->Format == VK_FORMAT_D32_SFLOAT_S8_UINT || VulkanImage->Format == VK_FORMAT_D24_UNORM_S8_UINT)
        {
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    cmdImageTransition(cmdBuffer, VulkanImage->Image, aspectMask, 
                        VulkanImage->CurrentAccess, dstAccesses, 
                        VulkanImage->CurrentLayout, dstLayout);

    // Update current layout, stages, and accesses
    VulkanImage->CurrentLayout = dstLayout;
    VulkanImage->CurrentAccess = dstAccesses;
}

