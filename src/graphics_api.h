#ifndef GRAPHICS_API
#define GRAPHICS_API

#include "game_platform.h"
#include "game_math.h"
#include "game_mesh.h"
#include "game_assets.h"

#define RENDER_API extern "C" GAME_API

/*
 * @fpVulkanCreateSurface       Pointer to OS specific function which creates the VkSurface handle 
 * @SurfaceData                 Pointer to OS specific data needed in fpVulkanCreateSurface
 * @VkKHROSSurfaceExtensionName Name of the khronos OS specific surface name which is needed to check support
 * @OSSurfaceFuncName           Name of the OS specific instance function name which we need the proc address
 */
struct graphics_platform_window
{
    vulkan_create_surface * pfnVulkanCreateSurface;
    void *                  SurfaceData;
    const char *            VkKHROSSurfaceExtensionName;
    const char *            OSSurfaceFuncName;
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
    v4 Color;
};

#define GRAPHICS_RENDER_DRAW(name) void name(render_controller * Renderer)
typedef GRAPHICS_RENDER_DRAW(graphics_render_draw);
RENDER_API
GRAPHICS_RENDER_DRAW(RenderDraw);

#define GRAPHICS_BEGIN_RENDER(name) void name(v4 ClearColor, GPUSimulationData * SimData)
typedef GRAPHICS_BEGIN_RENDER(graphics_begin_render);
RENDER_API 
GRAPHICS_BEGIN_RENDER(BeginRenderPass);

#define GRAPHICS_END_RENDER(name) i32 name()
typedef GRAPHICS_END_RENDER(graphics_end_render);
RENDER_API
GRAPHICS_END_RENDER(EndRenderPass);

#define GRAPHICS_PUSH_VERTEX_DATA(name) i32 name(void * Data, u32 DataSize, u32 * outVertexBufferBeginOffset)
typedef GRAPHICS_PUSH_VERTEX_DATA(graphics_push_vertex_data);
RENDER_API
GRAPHICS_PUSH_VERTEX_DATA(PushVertexData);

#define GRAPHICS_PUSH_TEXTURE_DATA(name) i32 name(void * Data, u32 Width, u32 Height, u32 Channels)
typedef GRAPHICS_PUSH_TEXTURE_DATA(graphics_push_texture_data);
RENDER_API
GRAPHICS_PUSH_TEXTURE_DATA(PushTextureData);

#define GRAPHICS_CREATE_SHADER_MODULE(name) i32 name(void * Buffer, size_t Size)
typedef GRAPHICS_CREATE_SHADER_MODULE(graphics_create_shader_module);
RENDER_API
GRAPHICS_CREATE_SHADER_MODULE(CreateShaderModule);

#define GRAPHICS_DELETE_SHADER_MODULE(name) void name(i32 ShaderIndex)
typedef GRAPHICS_DELETE_SHADER_MODULE(graphics_delete_shader_module);
RENDER_API
GRAPHICS_DELETE_SHADER_MODULE(DeleteShaderModule);

#if DEBUG
#define GRAPHICS_INITIALIZE_API(name) i32 name(debug_cycle * DebugCyclesArray, i32 Width, i32 Height, graphics_platform_window PlatformWindow, b32 EnableValidationLayer, PFN_vkGetInstanceProcAddr GetInstanceProcAddr)
#else
#define GRAPHICS_INITIALIZE_API(name) i32 name(i32 Width, i32 Height, graphics_platform_window PlatformWindow, b32 EnableValidationLayer, PFN_vkGetInstanceProcAddr GetInstanceProcAddr)
#endif
typedef GRAPHICS_INITIALIZE_API(graphics_initialize_api);
RENDER_API
GRAPHICS_INITIALIZE_API(InitializeAPI);

#define GRAPHICS_CLOSE_API(name) void name()
typedef GRAPHICS_CLOSE_API(graphics_close_api);
RENDER_API
GRAPHICS_CLOSE_API(ShutdownAPI);

#define GRAPHICS_WAIT_FOR_RENDER(name) i32 name()
typedef GRAPHICS_WAIT_FOR_RENDER(graphics_wait_for_render);
RENDER_API 
GRAPHICS_WAIT_FOR_RENDER(WaitForRender);

#define GRAPHICS_ON_WINDOW_RESIZE(name) i32 name(i32 Width,i32 Height)
typedef GRAPHICS_ON_WINDOW_RESIZE(graphics_on_window_resize);
RENDER_API
GRAPHICS_ON_WINDOW_RESIZE(OnWindowResize);

#define GRAPHICS_CREATE_MATERIAL_PIPELINE(name) pipeline_creation_result name(i32 VertexShaderIndex, i32 FragmentShaderIndex)
typedef GRAPHICS_CREATE_MATERIAL_PIPELINE(graphics_create_material_pipeline);
RENDER_API
GRAPHICS_CREATE_MATERIAL_PIPELINE(CreatePipeline);

#define GRAPHICS_CREATE_TRANSPARENCY_PIPELINE(name) transparency_pipeline_creation_result name(i32 VertexShaderIndex, i32 WeightFragmentShaderIndex,i32 FullscreenTriangleVertexShaderIndex, i32 CompositeFragmentShaderIndex)
typedef GRAPHICS_CREATE_TRANSPARENCY_PIPELINE(graphics_create_transparency_pipeline);
RENDER_API
GRAPHICS_CREATE_TRANSPARENCY_PIPELINE(CreateTransparencyPipeline);

#define GRAPHICS_DESTROY_MATERIAL_PIPELINE(name) void name(i32 Index)
typedef GRAPHICS_DESTROY_MATERIAL_PIPELINE(graphics_destroy_material_pipeline);
RENDER_API
GRAPHICS_DESTROY_MATERIAL_PIPELINE(FreeMaterialPipeline);


struct graphics_api
{
    graphics_render_draw                  * GraphicsRenderDraw;
    graphics_begin_render                 * GraphicsBeginRenderPass;
    graphics_end_render                   * GraphicsEndRenderPass;
    graphics_push_vertex_data             * GraphicsPushVertexData;
    graphics_push_texture_data            * GraphicsPushTextureData;
    graphics_initialize_api               * GraphicsInitializeApi;
    graphics_close_api                    * GraphicsShutdownAPI;
    graphics_wait_for_render              * GraphicsWaitForRender;
    graphics_on_window_resize             * GraphicsOnWindowResize;
    graphics_create_shader_module         * GraphicsCreateShaderModule;
    graphics_delete_shader_module         * GraphicsDeleteShaderModule;
    graphics_create_material_pipeline     * GraphicsCreateMaterialPipeline;
    graphics_destroy_material_pipeline    * GraphicsDestroyMaterialPipeline;
    graphics_create_transparency_pipeline * GraphicsCreateTransparencyPipeline;
};

#endif
