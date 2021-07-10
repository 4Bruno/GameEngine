#include "math.h"
#include "game.h"
#include "vulkan_initializer.h"

struct camera
{
    real32 Yaw;
    real32 Pitch;    
    v3 D;
};

m4
LookAtFromD(v3 D, v3 WorldUp)
{
    v3 Up = WorldUp - (Inner(WorldUp,D)*D);
    v3 Right = Cross(D,Up);

    m4 R = {};
    R[0].x = Right.x;
    R[1].x = Up.x;
    R[2].x = D.x;
    R[3].x = 0;

    R[0].y = Right.y;
    R[1].y = Up.y;
    R[2].y = D.y;
    R[3].y = 0;

    R[0].z = Right.z;
    R[1].z = Up.z;
    R[2].z = D.z;
    R[3].z = 0;

    R[0].w = 0;
    R[1].w = 0;
    R[2].w = 0;
    R[3].w = 1;

    return R;
}

m4
LookAt(v3 Position, v3 Target, v3 WorldUp)
{
    v3 D = (Target - Position) / Length(Target - Position);
    v3 Up = WorldUp - (Inner(WorldUp,D)*D);
    v3 Right = Cross(D,Up);

    m4 R = {};
    R[0].x = Right.x;
    R[1].x = Up.x;
    R[2].x = D.x;
    R[3].x = 0;

    R[0].y = Right.y;
    R[1].y = Up.y;
    R[2].y = D.y;
    R[3].y = 0;

    R[0].z = Right.z;
    R[1].z = Up.z;
    R[2].z = D.z;
    R[3].z = 0;

    R[0].w = 0;
    R[1].w = 0;
    R[2].w = 0;
    R[3].w = 1;

    return R;
}
