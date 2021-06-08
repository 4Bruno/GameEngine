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
};

#define MESH_H
#endif
