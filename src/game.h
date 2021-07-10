#ifndef GAME_H
#include "game_platform.h"
#include "mesh.h"

#define PushSize(Arena,Size) _PushSize(Arena,Size*sizeof(char))
#define PushArray(Arena,Count,Struct) (Struct *)_PushSize(Arena,Count*sizeof(Struct))
#define PushStruct(Arena,Struct) (Struct *)PushArray(Arena,1,Struct)

#define DEFAULT_WORLD_UP V3(0,1,0)
struct renderer_3d
{
    v3  WorldUp;

    m4 Projection;

    m4 ViewMoveMatrix;
    m4 ViewRotationMatrix;
    m4 WorldTransform;
    
    m4 ObjectMoveMatrix;
    m4 ObjectRotationMatrix;
    m4 ObjectTransform;
    m4 ObjectScale;

    m4 MVP;
};

inline uint8 *
_PushSize(memory_arena * Arena,uint32 Size)
{
    Assert((Arena->CurrentSize + Size) < Arena->MaxSize);
    uint8 * BaseAddr = Arena->Base + Arena->CurrentSize;
    Arena->CurrentSize += Size;
    return BaseAddr;
}


struct entity
{
    v3 D;
    v3 P;
    real32 dP;
    mesh * Mesh;
    real32 Height;
    v3 Scale;
};


struct game_state;
struct scene;

#define SCENE_LOADER(name) void name(game_state * GameState, scene * Scene)
typedef SCENE_LOADER(scene_loader);

#define SCENE_HANDLER(name) void name(game_state * GameState,game_input * Input, scene * Scene)
typedef SCENE_HANDLER(scene_handler);

struct scene
{
    scene_loader  * Loader;
    scene_handler * Handler;
    bool32          Loaded;
    entity * Entities[10];
    uint32 EntityCount;
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


struct game_state
{
    bool32 IsInitialized;

    memory_arena TemporaryArena;
    memory_arena ShadersArena;
    memory_arena MeshesArena;
    memory_arena VertexArena;
    memory_arena IndicesArena;

    int32 PipelineIndex;
    int32 VertexShaders[2];
    int32 FragmentShaders[2];

    entity Entities[100];
    uint32 TotalEntities;

    entity * Player;

    mesh Meshes[10];
    uint32 TotalMeshes;

    scene Scenes[2];
    uint32 CurrentScene;

    renderer_3d Renderer;


    camera Camera;
    v3 CameraP;
    real32 DebugLastTimeStamp;
    v3 DebugSourceLight;
};



#define GAME_H
#endif
