#ifndef MESH_H

#include "game_platform.h"
#include "math.h"

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

#define MESH_H
#endif
