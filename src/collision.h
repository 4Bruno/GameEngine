#ifndef COLLISION_H

#include "game_platform.h"
#include "game_mesh.h"

struct AABB
{
    v3 c; // center
    v3 r; // radius (halfway)
};

enum collision_type
{
    collision_aabb
};

struct collision
{
    collision_type CollisionType;
};

bool32
CollisionTestAABBAABB(AABB a, AABB b);

void 
ExtremePointsAlongDirection(v3 dir, vertex_point * pt, int n, int *imin, int *imax);

#define COLLISION_H
#endif
