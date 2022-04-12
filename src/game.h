#ifndef GAME_H

#include "game_platform.h"
#if DEBUG
extern "C" debug_cycle * DebugCycles;
#endif

#include "game_random.h"
#include "game_memory.h"
#include "game_entity.h"
#include "game_world.h"
#include "game_mesh.h"
#include "game_render.h"
#include "game_platform.h"
#include "game_animation.h"
#include "game_assets.h"

#include "collision.h"
#include "Quaternion.h"

#define NULL_MESH   u32_MAX
#define VALID_MESH_ID(ID) (ID != NULL_MESH)

#define INVALID_MESHOBJ_TRANSFORM_INDEX UINT32_MAX
#define IS_VALID_MESHOBJ_TRANSFORM_INDEX(IDX) (IDX < INVALID_MESHOBJ_TRANSFORM_INDEX)


#define RGB_RED   V3(1.0f,0,0)
#define RGB_GREEN V3(0.0f,1.0f,0)
#define RGB_BLUE  V3(0.0f,0.0f,1.0f)
#define RGB_WHITE V3(1.0f,1.0f,1.0f)
#define RGB_GREY  V3(0.5f,0.5f,0.5f)

#define LOG_P(P) Log("x: %f y: %f z: %f \n", P.x, P.y, P.z);

struct sphere
{
    v3 c;
    r32 r;
};


struct scene;

#define SCENE_LOADER(name) void name(game_state * GameState, i32 ScreenX, i32 ScreenY)
typedef SCENE_LOADER(scene_loader);

#define SCENE_HANDLER(name) void name(game_state * GameState,game_input * Input,v3 dP, r32 Yaw, r32 Pitch, i32 ScreenX, i32 ScreenY)
typedef SCENE_HANDLER(scene_handler);


struct scene
{
    scene_loader  * Loader;
    scene_handler * Handler;
    b32          Loaded;
    entity * Entities[10];
    u32 EntityCount;
    entity * LightSources[1];
};

struct mouse_drag
{
    v2 StartP;
    r32 StartTime;
};

struct camera
{
    r32 Yaw;
    r32 Pitch;    
    v3 D;
};

struct render_3D
{
    u32 MeshID;
    v3 Color;
};

struct bucket_world_pos
{
    world_pos WorldP;
    entity Entity;
    bucket_world_pos * Next;
};

struct menu_overlay
{
    u32 CountAnchoredTop;
    u32 CountAnchoredBottom;
};

struct particle_cell
{
    r32 Density;
    v3 VelocityTimesDensity;
};

struct particle
{
    entity_transform T;
    v3 dP;
    v3 ddP;
    v3 Color;
    r32 DistToCamera;
};

struct game_state
{
    b32 IsInitialized;

    memory_arena PermanentArena;
    memory_arena TemporaryArena;

    memory_arena RenderArena;
    memory_arena WorldArena;
    world World;

    simulation * Simulation;

    thread_memory_arena * ThreadArena;
    u32 LimitThreadArenas;

#if 0
    mesh_group * Meshes;
    u32 LimitMeshes;

    mesh_group * GroundMeshGroup;
    u32 GroundMeshLimit;
    u32 GroundMeshCount;
    u32 MaxGroundByteSize;
#endif

    game_assets Assets;
    render_controller Renderer;
    render_controller RendererUI;
    menu_overlay DebugOverlay;

    world_pos CameraWorldP;
    b32 CameraMode;
    camera Camera;

    animation DebugAnimate;
    v3 DebugSourceLightP;

#define PARTICLE_CELL_DIM 16 
#define PARTICLE_CELL_DIM_MINUS_ONE (PARTICLE_CELL_DIM - 1)
#define PARTICLE_CELL_DIM_MINUS_TWO (PARTICLE_CELL_DIM - 2)
    particle Particles[256];
    vertex_point ParticlePoints[256*6];
    particle_cell ParticleCells[PARTICLE_CELL_DIM][PARTICLE_CELL_DIM][PARTICLE_CELL_DIM];
    u32 NextParticle;
    mesh_group ParticlesMesh;

    random_series RandomSeed;
};

extern graphics_render_draw * GraphicsRenderDraw ;
extern graphics_begin_render * GraphicsBeginRenderPass ;
extern graphics_end_render * GraphicsEndRenderPass ;
extern graphics_push_vertex_data * GraphicsPushVertexData ;
extern graphics_push_texture_data * GraphicsPushTextureData ;
extern graphics_initialize_api * GraphicsInitializeApi ;
extern graphics_close_api * GraphicsShutdownAPI ;
extern graphics_wait_for_render * GraphicsWaitForRender ;
extern graphics_on_window_resize * GraphicsOnWindowResize ;
extern graphics_create_shader_module * GraphicsCreateShaderModule;
extern graphics_delete_shader_module * GraphicsDeleteShaderModule;
extern graphics_create_material_pipeline * GraphicsCreateMaterialPipeline;
extern graphics_destroy_material_pipeline * GraphicsDestroyMaterialPipeline;
extern graphics_create_transparency_pipeline * GraphicsCreateTransparencyPipeline;

extern game_memory * GlobalPlatformMemory;
extern game_assets * GlobalAssets;


#define GAME_H
#endif
