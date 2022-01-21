#ifndef GAME_H

#include "vulkan_initializer.h"
#include "game_platform.h"
#include "game_memory.h"
#include "game_entity.h"
#include "game_world.h"
#include "game_mesh.h"
#include "game_render.h"
#include "game_platform.h"
#include "game_animation.h"

#include "data_load.h"
#include "collision.h"
#include "Quaternion.h"

#define NULL_MESH   u32_MAX
#define VALID_MESH_ID(ID) (ID != NULL_MESH)

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

struct game_state
{
    b32 IsInitialized;

    memory_arena PermanentArena;
    memory_arena TemporaryArena;
    memory_arena ShadersArena;
    memory_arena MeshesArena;
    memory_arena VertexArena;
    memory_arena IndicesArena;

    memory_arena WorldArena;
    world World;

    simulation * Simulation;

    thread_memory_arena * ThreadArena;
    u32 LimitThreadArenas;

    i32 PipelineIndex;
    i32 VertexShaders[2];
    i32 FragmentShaders[2];

    mesh_group * Meshes;
    u32 LimitMeshes;
    mesh_group GroundMeshGroup;

    v3 WorldUp;
    m4 ViewMoveMatrix;
    m4 ViewRotationMatrix;
    m4 ViewTransform;
    m4 Projection;

    b32 CameraMode;
    camera Camera;

    animation DebugAnimate;
};



#define GAME_H
#endif
