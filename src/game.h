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

    entity Entities[10];
    uint32 TotalEntities;

    mesh Meshes[10];
    uint32 TotalMeshes;

    v3 CameraP;
};

#define GAME_H
#endif
