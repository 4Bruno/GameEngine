#ifndef GAME_MESH_H
#include "game_platform.h"
#include "game_memory.h"
#include "game_math.h"


#define INVALID_MESHID UINT32_MAX
#define IS_VALID_MESHID(ID) (ID < UINT32_MAX)

#define MAX_MESH_COUNT 3

struct vertex_point 
{
    v3 P;
    v3 N;
    v4 Color;
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

    b32 Loaded;
    b32 LoadInProcess;
};

struct async_load_mesh
{
    game_memory * Memory;
    game_state * GameState;
    char * Path;
    mesh_group  * MeshGroup;
    thread_memory_arena * ThreadArena;
    u32 BaseOffset;
};

GAME_API mesh_group *
GetMesh(game_memory * Memory, game_state * GameState,u32 ID);
GAME_API u32
PushMeshSize(memory_arena * Arena, u32 DataSize, u32 InstanceCount);

#define GAME_MESH_H
#endif
