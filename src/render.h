#ifndef RENDER_H
#include "game_platform.h"
#include "mesh.h"

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

struct mesh_push_constant
{
    v4 Data;
    m4 RenderMatrix;
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
RenderLoop(int32 PipelineIndex, uint32 TotalMeshInstances, uint32 MeshSize);

RENDER_API
int32
RenderCreateShaderModule(char * Buffer, size_t Size);

RENDER_API
int32
RenderCreatePipeline(int32 VertexShaderIndex,
                     int32 FragmentShaderIndex);
RENDER_API
int32
RenderBeginPass(v4 ClearColor);

RENDER_API
int32
RenderSetPipeline(int32 PipelineIndex);

RENDER_API
int32
RenderPushMesh(uint32 TotalMeshInstances, uint32 MeshSize, uint32 IndicesSize = 0, VkDeviceSize OffsetVertex = 0, VkDeviceSize OffsetIndices = 0);

RENDER_API
int32
RenderEndPass();

RENDER_API
int32
RenderPushVertexData(memory_arena * Arena,void * Data,uint32 DataSize,uint32 InstanceCount);

RENDER_API
int32
RenderPushIndexData(memory_arena * Arena,void * Data,uint32 DataSize,uint32 InstanceCount);

RENDER_API
memory_arena
RenderGetMemoryArena();

RENDER_API
void
RenderPushVertexConstant(uint32 Size,void * Data);

#define RENDER_H
#endif
