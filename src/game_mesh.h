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

    u16       * Indices;
    u32         IndicesSize;

    u32 OffsetVertices;
    u32 OffsetIndices;

    b32 Loaded;
    b32 LoadInProcess;
};

struct async_load_mesh
{
    game_memory * Memory;
    game_state * GameState;
    char * Path;
    mesh * Mesh;
    thread_memory_arena * ThreadArena;
    void * BufferVertices;
    u32 BaseOffset;
};

mesh *
GetMesh(game_memory * Memory, game_state * GameState,u32 ID);

#define GAME_MESH_H
#endif
