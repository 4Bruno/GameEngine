#ifndef GAME_RENDER_H

#define DEFAULT_WORLD_UP V3(0,1,0)
#include "game_math.h"
#include "game_entity.h"

enum  shader_type_vertex
{
    shader_type_vertex_default,
    shader_type_vertex_texture,


    shader_type_vertex_total
};

enum  shader_type_fragment
{
    shader_type_fragment_default,
    shader_type_fragment_texture,


    shader_type_fragment_total
};


enum enum_textures
{
    enum_texture_none = 0,
    enum_texture_ground_stone = 1
};




struct render_unit
{
    m4 ModelTransform;
    v4 Color;
    mesh_id MeshID;
};

struct render_pass
{
    render_unit * Units;
    u32 Limit;
    u32 Count;
};

struct gpu_arena
{
    u32 TotalSize;
    u32 CurrentSize;
    i32 MemoryAlign;
};

struct render_controller
{
    v3 WorldUp;
    m4 ViewMoveMatrix;
    m4 ViewRotationMatrix;
    m4 ViewTransform;
    m4 Projection;

    memory_arena * Arena;

    i32 Pipelines[16];
    i32 VertexShaders[2];
    i32 FragmentShaders[2];

#if DEBUG
    render_pass RenderPassDebug;
#endif

    render_pass RenderPasses[2];

    // This arena has no memory backup
    // it only tracks GPU vertex buffer offsets
    memory_arena VertexArena;
    memory_arena IndicesArena;
    memory_arena TextureArena;
};

GAME_API void
DestroyPipelines(game_state * GameState,render_controller * Renderer);
GAME_API void
CreateAllPipelines(game_state * GameState, game_memory * Memory);

GAME_API void
PushDrawSimulation(game_memory * Memory, game_state * GameState, simulation * Sim);

GAME_API pipeline_creation_result
CreatePipeline(game_memory * Memory,memory_arena * TempArena,render_controller * Renderer,material_type Material);

GAME_API void
EntityAdd3DRender(render_controller * Renderer, entity Entity, u32 MeshID, v3 Color);

GAME_API void
BeginRender(render_controller * Renderer, v4 ClearColor);

GAME_API void
EndRender(render_controller * Renderer);

GAME_API void
RenderEntities(game_memory * Memory, render_controller * Renderer);
GAME_API void
RenderEntity(render_controller * Renderer,game_memory * Memory,entity * Entity);

#if DEBUG
GAME_API void
PushDrawDebug(render_controller * Renderer,entity * Entity);
#endif

GAME_API void
PushDrawEntity(render_controller * Renderer,entity * Entity);

GAME_API render_controller
NewRenderController(memory_arena * Arena,
                    v3 WorldUp,
                    r32 FOV,
                    i32 ScreenWidth, i32 ScreenHeight, 
                    r32 n, r32 f,
                    v3 StartP);

GAME_API void 
RotateFill(m4 * M, r32 AngleX, r32 AngleY, r32 AngleZ);

GAME_API void
MoveViewRight(render_controller * Renderer,r32 N);

GAME_API void
MoveViewForward(render_controller * Renderer,r32 N);


GAME_API inline v3
GetViewPos(render_controller * Renderer);

GAME_API inline v3
GetMatrixRight(m4 &RotationMatrix);

GAME_API inline v3
GetMatrixPos(m4 &M);

GAME_API inline v3
GetMatrixDirection(m4 &RotationMatrix);

i32
ViewLookAt(render_controller * Renderer, v3 P, v3 TargetP);

GAME_API void
EntityLookAt(render_controller * Renderer,entity Entity, v3 P);

m4
ProjectionMatrix(r32 FOV,r32 AspectRatio, r32 n, r32 f);

GAME_API void
RenderDraw(game_state * GameState, game_memory * Memory,render_controller * Renderer);

GAME_API void
RenderDrawGround(game_state * GameState,render_controller * Renderer, simulation * Sim);

GAME_API void
GetTexture(game_state * GameState,game_memory * Memory,memory_arena * Arena, enum_textures TextureID);

#define GAME_RENDER_H
#endif
