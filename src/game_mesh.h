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
    uint32         VertexSize;

    uint16       * Indices;
    uint32         IndicesSize;

    uint32 OffsetVertices;
    uint32 OffsetIndices;

    bool32 Loaded;
    bool32 LoadInProcess;
};

struct async_load_mesh
{
    game_memory * Memory;
    game_state * GameState;
    char * Path;
    mesh * Mesh;
    thread_memory_arena * ThreadArena;
    void * BufferVertices;
    uint32 BaseOffset;
};

GAME_API mesh *
GetMesh(game_memory * Memory, game_state * GameState,uint32 ID);

#define GAME_MESH_H
#endif
