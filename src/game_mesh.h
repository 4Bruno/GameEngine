#ifndef GAME_MESH_H
#include "game_platform.h"
#include "game_memory.h"
#include "game_math.h"

struct vertex_point 
{
    v3 P;
    v3 N;
    v4 Color;
};

struct mesh
{
    vertex_point * Vertices;
    u32         VertexSize;

    u32 OffsetVertices; 
    u32 OffsetIndices;

    u16       * Indices;
    u32         IndicesSize;
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
    void * BufferVertices;
    u32 BaseOffset;
};

GAME_API mesh_group *
GetMesh(game_memory * Memory, game_state * GameState,u32 ID);
GAME_API void
PushMeshSize(memory_arena * Arena, u32 DataSize, u32 InstanceCount);

#define GAME_MESH_H
#endif
