#ifndef VULKAN_INITIALIZER_H
#include "game_platform.h"
#include "game_memory.h"
#include "game_mesh.h"

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

//#include "C:/Users/48793/resources/glm/glm/mat4x4.hpp" // glm::mat4

struct mesh_push_constant
{
    m4 Model;
    v4 DebugColor;
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
    v4 Color;
};

struct pipeline_creation_result
{
    b32 Success;
    i32 Pipeline;
    i32 PipelineLayout;
};

RENDER_API i32
RenderPushSimulationData(GPUSimulationData * SimData);

RENDER_API
i32
InitializeVulkan(i32 Width, i32 Height, 
                 vulkan_platform_window PlatformWindow,
                 b32 EnableValidationLayer,
                 PFN_vkGetInstanceProcAddr GetInstanceProcAddr);
RENDER_API
void
CloseVulkan();

RENDER_API
GPUObjectData *
VulkanBeginObjectDataMapping(u32 * CurrentObjectCount);

RENDER_API
void
VulkanEndObjectDataMapping(u32 ObjectCount);

RENDER_API
i32
VulkanOnWindowResize(i32 Width,i32 Height);

RENDER_API
i32
WaitForRender();

RENDER_API
i32
RenderCreateShaderModule(char * Buffer, size_t Size);
RENDER_API
i32
RenderFreeShaders();

RENDER_API
pipeline_creation_result
RenderCreatePipeline(i32 VertexShaderIndex,
                     i32 FragmentShaderIndex);
RENDER_API
i32
RenderBeginPass(v4 ClearColor);

RENDER_API
i32
RenderSetPipeline(i32 PipelineIndex);

RENDER_API
i32
RenderDrawObject(u32 VertexSize,u32 FirstInstance);

RENDER_API
i32
RenderPushMeshIndexed(u32 TotalMeshInstances, u32 IndicesSize, VkDeviceSize OffsetVertex = 0, VkDeviceSize OffsetIndices = 0);

RENDER_API
i32
RenderEndPass();

RENDER_API
u32
RenderGetVertexMemAlign();

RENDER_API
i32
RenderPushVertexData(void * Data, u32 DataSize, u32 BaseOffset);

RENDER_API
i32
RenderPushIndexData(void * Data, u32 DataSize, u32 BaseOffset);

RENDER_API
memory_arena
RenderGetMemoryArena();

RENDER_API
i32
RenderBindMesh(u32 VertexSize, u32 Offset);

RENDER_API
void
RenderPushVertexConstant(u32 Size,void * Data);

RENDER_API i32
VulkanPushTexture(void * Data, u32 Width, u32 Height, u32 BaseOffset, u32 Channels);

RENDER_API
void
VulkanWaitForDevices();

RENDER_API
void
VulkanDestroyPipeline();


RENDER_API
i32
RenderBindMaterial(u32 PipelineIndex);

#define VULKAN_INITIALIZER_H
#endif
