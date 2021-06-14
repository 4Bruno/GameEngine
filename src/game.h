#ifndef GAME_H
#include "game_platform.h"
#include "mesh.h"

#define PushSize(Arena,Size) _PushSize(Arena,Size*sizeof(char))
#define PushArray(Arena,Count,Struct) (Struct *)_PushSize(Arena,Count*sizeof(Struct))
#define PushStruct(Arena,Struct) (Struct *)PushArray(Arena,1,Struct)

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
    v3 P;
    mesh * Mesh;
    real32 Height;
    v3 Scale;
};


struct game_state;

#define SCENE_LOADER(name) void name(game_state * GameState)
typedef SCENE_LOADER(scene_loader);

#define SCENE_HANDLER(name) void name(game_state * GameState,game_input * Input,m4 Proj, m4 View)
typedef SCENE_HANDLER(scene_handler);

struct scene
{
    scene_loader  * Loader;
    scene_handler * Handler;
    bool32          Loaded;
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

    mesh Meshes[10];
    uint32 TotalMeshes;

    scene Scenes[2];
    uint32 CurrentScene;

    v3 CameraP;
};


#define GAME_H
#endif
