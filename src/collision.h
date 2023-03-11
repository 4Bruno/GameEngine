#ifndef COLLISION_H

#include "game_platform.h"
#include "game_mesh.h"
#include "game_assets.h"


enum bounding_volume_type
{
    bounding_volume_aabb,   /* Axis-aligned Bounding Boxes */
    bounding_volume_sphere,
    bounding_volume_obb     /* Oriented Bounding Box */

};

b32
CollisionTestAABBAABB(AABB a, AABB b);

void 
ExtremePointsAlongDirection(v3 dir, vertex_point * pt, int n, int *imin, int *imax);

//void SphereFromDistantPoints(sphere * S, vertex_point * Vertices, u32 VertexCount);
void SphereFromDistantPoints(sphere * s, vertex_point * Vertices, u32 VertexCount);

b32
TestCollisionSpheres(sphere s0, sphere s1, v3 dP0, v3 dP1, r32 & t);

#define COLLISION_H
#endif
