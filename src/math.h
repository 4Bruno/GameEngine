#ifndef MATH_H
#include "game_platform.h"
#include <math.h>

struct v2
{
    union
    {
        real32 _V[2];
        struct
        {
            real32 x,y;
        };
    };
};

struct v3
{
    union
    {
        real32 _V[3];
        struct
        {
            real32 x,y,z;
        };
        struct
        {
            v2 xy;
            real32 _Unused00;
        };
        struct
        {
            real32 _Unused01;
            v2 yz;
        };
    };
};

struct v4
{
    union
    {
        real32 _V[4];
        struct
        {
            real32 x,y,z,w;
        };
        struct
        {
            real32 r,g,b,a;
        };
        struct
        {
            v2 xy;
            real32 _Unused00,_Unused01;
        };
        struct
        {
            v3 xyz;
            real32 _Unused02;
        };
        struct
        {
            real32 _Unused03;
            v2 yzw;
        };
    };
};

inline v3
V3(real32 x, real32 y, real32 z)
{
    v3 Result = { x, y, z};
    return Result;
}
v3
operator +(v3 A, v3 B)
{
    v3 Result = { A.x + B.x, A.y + B.y, A.z + B.z };

    return Result;
}

v3
operator +=(v3 &A, v3 B)
{
    A = A + B;

    return A;
}

v3
operator *(v3 A, real32 B)
{
    v3 Result = {A.x * B, A.y * B, A.z * B};

    return Result;
}

v3
operator *=(v3 &A, real32 B)
{
    A = A*B;

    return A;
}

inline v4
V4(real32 x, real32 y, real32 z, real32 w)
{
    v4 Result = { x, y, z, w};
    return Result;
}


























#define MATH_H
#endif
