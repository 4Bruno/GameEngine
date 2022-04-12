#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include "game_platform.h"
#include "game_memory.h"
#include "game_mesh.h"
#include "game_math.h"


struct pipeline_creation_result
{
    b32 Success;
    i32 Pipeline;
    i32 PipelineLayout;
};

struct transparency_pipeline_creation_result
{
    pipeline_creation_result PipelineCreationResult[2];
};


enum asset_state
{
    asset_unloaded,
    asset_load_error,
    asset_load_inprogress,
    asset_loaded_on_cpu,
    asset_transfer_to_gpu_inprogress,
    asset_loaded_on_gpu,
    asset_loaded,
    asset_loaded_locked
};

struct asset_slot
{
    asset_state State;    
    void * Data;
    size_t Size;
};


#define ASSETS_DEFAULT_MESH     (game_asset_id)(game_asset_mesh_begin + 1)
#define ASSETS_DEFAULT_MATERIAL (game_asset_id)(game_asset_material_begin + 2)
#define ASSETS_NULL_TEXTURE     ((game_asset_id)-1)
#define ASSETS_TOTAL_MESHES     (game_asset_mesh_end - game_asset_mesh_begin - 1)
#define ASSETS_TOTAL_MATERIALS  (game_asset_material_end - game_asset_material_begin - 1)
#define ASSETS_TOTAL_SHADERS    (game_asset_shader_end - game_asset_shader_begin - 1)
#define ASSETS_TOTAL_TEXTURES   (game_asset_texture_end - game_asset_texture_begin - 1)
#define ASSETS_TOTAL_COUNT (ASSETS_TOTAL_MESHES + ASSETS_TOTAL_MATERIALS + ASSETS_TOTAL_SHADERS + ASSETS_TOTAL_TEXTURES)

enum game_asset_id
{
    /* MESHES */
    game_asset_mesh_begin,

    game_asset_mesh_cube,
    game_asset_mesh_quad,

    game_asset_mesh_end,

    /* SHADERS */
    game_asset_shader_begin,

    game_asset_shader_vertex_default_no_light,
    game_asset_shader_vertex_default_light,
    game_asset_shader_vertex_fullscreen_triangle,
    game_asset_shader_fragment_default,
    game_asset_shader_fragment_texture,
    game_asset_shader_fragment_oit_weighted_color,
    game_asset_shader_fragment_oit_weighted_composite,

    game_asset_shader_end,

    /* MATERIALS */
    game_asset_material_begin,

    game_asset_material_default_no_light,
    game_asset_material_default_light,
    game_asset_material_texture_no_light,
    game_asset_material_texture,
    game_asset_material_transparent,

    game_asset_material_end,

    /* TEXTURES */
    game_asset_texture_begin,
    game_asset_texture_test,
    game_asset_texture_particle_01_small,
    game_asset_texture_end,

};

struct vertex_point 
{
    v3 P;
    v3 N;
    v4 Color;
    v2 UV;
};


struct mesh
{
    //vertex_point * Vertices;
    //u16       * Indices;
    u32 VertexSize;
    u32 IndicesSize;
    
    u32 OffsetVertices;
    u32 OffsetIndices;
};

struct mesh_group
{
    mesh * Meshes;
    u32 TotalMeshObjects;
    u32 GPUVertexBufferBeginOffset;
    game_asset_id AssetID;
};

#if 0
struct platform_api
{
        
};
#endif

struct asset_shader
{
    i32 GPUID;    
    game_asset_id AssetID;
};

struct asset_texture
{
    i32 GPUID;    
    i32 Width, Height, Channels;
    game_asset_id AssetID;
};

struct asset_material
{
    pipeline_creation_result Pipeline[2];
    u32 PipelinesCount;
    game_asset_id AssetID;
};

struct game_assets
{
    memory_arena SoundArena;

    memory_arena ShaderArena;
    u32 ShaderLoadInProgress;

    memory_arena MeshArena;
    u32 MeshArenaPermanentSize;
    u32 MeshLoadInProgress;

    // power of 2
    asset_shader CachedShaders[16];
    u32 CachedShadersIndex;

    mesh_group CachedMeshGroups[256];
    u32 CachedMeshGroupIndex;

    asset_texture CachedTextures[256];
    u32 CachedTexturesIndex;

    asset_material CachedMaterials[16];
    u32 CachedMaterialIndex;

    thread_memory_arena * ThreadArena;
    i32 LimitThreadArenas;

    asset_slot * AssetSlots;   
};


asset_material *
GetMaterial(game_assets * Assets, game_asset_id MaterialID);

mesh_group *
GetMesh(game_assets * Assets, game_asset_id ID);

asset_texture *
GetTexture(game_assets * Assets, game_asset_id ID);

game_assets
NewGameAssets(memory_arena * Arena, thread_memory_arena * ThreadArenas, i32 LimitThreadArenas);


#endif
