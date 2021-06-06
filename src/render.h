#ifndef RENDER_H
#include "game_platform.h"

#define RENDER_API __declspec( dllexport )

/*
 * @fpVulkanCreateSurface       Pointer to OS specific function which creates the VkSurface handle 
 * @SurfaceData                 Pointer to OS specific data needed in fpVulkanCreateSurface
 * @VkKHROSSurfaceExtensionName Name of the khronos OS specific surface name which is needed to check support
 * @OSSurfaceFuncName           Name of the OS specific instance function name which we need the proc address
 */
struct vulkan_platform_window
{
    vulkan_create_surface * pfnVulkanCreateSurface;
    void *                  SurfaceData;
    const char *            VkKHROSSurfaceExtensionName;
    const char *            OSSurfaceFuncName;
};

RENDER_API
int32
InitializeVulkan(int32 Width, int32 Height, 
                 vulkan_platform_window PlatformWindow,
                 bool32 EnableValidationLayer,
                 PFN_vkGetInstanceProcAddr GetInstanceProcAddr);
RENDER_API
void
CloseVulkan();

RENDER_API
int32
VulkanOnWindowResize(int32 Width,int32 Height);

RENDER_API
int32
WaitForRender();

RENDER_API
int32
RenderLoop(real32 TimeElapsed, int32 PipelineIndex);

RENDER_API
int32
RenderCreateShaderModule(char * Buffer, size_t Size, VkShaderModule * ShaderModule);

RENDER_API
int32
RenderCreatePipeline(VkShaderModule VertexShader,
                     VkShaderModule FragmentShader);

#define RENDER_H
#endif
