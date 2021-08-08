#ifndef GAME_H

#include "vulkan_initializer.h"
#include "game_platform.h"
#include "game_memory.h"
#include "game_entity.h"
#include "game_mesh.h"
#include "game_render.h"

#include "data_load.h"
#include "collision.h"
#include "Quaternion.h"

#define NULL_MESH   UINT32_MAX
#define VALID_MESH_ID(ID) (ID != NULL_MESH)

#define RGB_RED   V3(1.0f,0,0)
#define RGB_GREEN V3(0.0f,1.0f,0)
#define RGB_BLUE  V3(0.0f,0.0f,1.0f)
#define RGB_WHITE V3(1.0f,1.0f,1.0f)
#define RGB_GREY  V3(0.5f,0.5f,0.5f)


struct sphere
{
    v3 c;
    real32 r;
};


struct scene;

#define SCENE_LOADER(name) void name(game_state * GameState, int32 ScreenX, int32 ScreenY)
typedef SCENE_LOADER(scene_loader);

#define SCENE_HANDLER(name) void name(game_state * GameState,game_input * Input,v3 dP, real32 Yaw, real32 Pitch, int32 ScreenX, int32 ScreenY)
typedef SCENE_HANDLER(scene_handler);


struct scene
{
    scene_loader  * Loader;
    scene_handler * Handler;
    bool32          Loaded;
    entity * Entities[10];
    uint32 EntityCount;
    entity * LightSources[1];
};

struct mouse_drag
{
    v2 StartP;
    real32 StartTime;
};

struct camera
{
    real32 Yaw;
    real32 Pitch;    
    v3 D;
};

enum component_flags
{
    component_none = 0 << 0,

    component_input     = 1 << 0,
    component_collision = 1 << 1,
    component_render_3d = 1 << 2,
    component_transform = 1 << 3,
    component_momentum  = 1 << 4,
    component_ground    = 1 << 5

};


struct render_3D
{
    uint32 MeshID;
    v3 Color;
};

struct entity_input
{
    v3 dP;
};


struct game_state
{
    bool32 IsInitialized;

    memory_arena PermanentArena;
    memory_arena TemporaryArena;
    memory_arena ShadersArena;
    memory_arena MeshesArena;
    memory_arena VertexArena;
    memory_arena IndicesArena;

    thread_memory_arena * ThreadArena;
    uint32 LimitThreadArenas;

    int32 PipelineIndex;
    int32 VertexShaders[2];
    int32 FragmentShaders[2];

    entity * Entities;
    uint32 LimitEntities;
    uint32 TotalEntities;

    component_flags * EntitiesFlags;

    entity * EntitiesParent;

    // Component position/rotation/scale
    entity_transform * EntitiesTransform;

    // Component Input
    entity_input * EntitiesInput;
    entity_input * EntitiesMomentum;

    // Component collision
    collision * Collisions;

    // Component rendering
    render_3D * Render3D;

    mesh * Meshes;
    uint32 LimitMeshes;

    entity Player;

    v3 WorldUp;
    m4 ViewMoveMatrix;
    m4 ViewRotationMatrix;
    m4 ViewTransform;
    m4 Projection;

    bool32 CameraMode;
    camera Camera;

    scene_loader * CurrentSceneLoader;
    scene_handler * CurrentSceneHandler;
    void * SceneData;
    bool32 SceneLoaded;


    Quaternion DebugOrientation;

};



#define GAME_H
#endif
