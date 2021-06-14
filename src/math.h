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
operator -(v3 A, v3 B)
{
    v3 Result = { A.x - B.x, A.y - B.y, A.z - B.z };

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

struct m4
{
    
    union
    {
        real32 _V[16];
        real32 _RC[4][4];
        struct
        {
            real32 r00,r01,r02,r03;
            real32 r10,r11,r12,r13;
            real32 r20,r21,r22,r23;
            real32 r30,r31,r32,r33;
        };
        struct
        {
            real32 c00,c10,c20,c30;
            real32 c01,c11,c21,c31;
            real32 c02,c12,c22,c32;
            real32 c03,c13,c23,c33;
        };
        struct
        {
            v4 r0;
            v4 r1;
            v4 r2;
            v4 r3;
        };
    };
};

m4 
IdentityMatrix()
{
    m4 m = {};
    m.r00 = 1.0f;
    m.r11 = 1.0f;
    m.r22 = 1.0f;
    m.r33 = 1.0f;

    return m;
}

m4 
ScaleMatrix(real32 x, real32 y, real32 z)
{
    m4 m = {};
    m.r00 = x;
    m.r11 = y;
    m.r22 = z;
    m.r33 = 1.0f;

    return m;
}
m4 
ScaleMatrix(v3 A)
{
    return ScaleMatrix(A.x,A.y,A.z);
}



m4
ProjectionMatrix(real32 AngleOfView, real32 n, real32 f)
{
    real32 OneOverFN= 1.0f / (f - n);
    real32 S = (real32)(1.0 / (tan((AngleOfView * 0.5f) * (PI * (1.0f / 180.0f)))));
    m4 Result = {
        S,0,0,0,
        0,S,0,0,
        0,0,-f * OneOverFN, -1,
        0,0,(-f * n) * OneOverFN,0
    };
    return Result;
}
m4
Transpose(m4 A)
{
    m4 Result = {
        A.c00, A.c01, A.c02, A.c03,
        A.c10, A.c11, A.c12, A.c13,
        A.c20, A.c21, A.c22, A.c23,
        A.c30, A.c31, A.c32, A.c33
    };

    return Result;
}
m4
RotationMatrix(real32 AngleOfView, v3 Axis)
{
    real32 C = (real32)cos(AngleOfView);
    real32 S = (real32)sin(AngleOfView);
    m4 Result = {};
    if (Axis.z)
    {
        Result = {
            C,S,0,0,
            -S,C,0,0,
            0,0,1,0,
            0,0,0,1
        };
    } 
    else if (Axis.x)
    {
        Result = {
            1,0,0,0,
            0,C,S,0,
            0,-S,C,0,
            0,0,0,1
        };
    }
    else if (Axis.y)
    {
        Result = {
            C,0,-S,0,
            0,1,0,0,
            S,0,C,0,
            0,0,0,1
        };
    }
    return Result;
}

v4
operator *(v4 A, m4 B)
{
    v4 Result = {
        (A.x * B.r00 + A.y * B.r10 + A.z * B.r20 + A.w * B.r30),
        (A.x * B.r01 + A.y * B.r11 + A.z * B.r21 + A.w * B.r31),
        (A.x * B.r02 + A.y * B.r12 + A.z * B.r22 + A.w * B.r32),
        (A.x * B.r03 + A.y * B.r13 + A.z * B.r23 + A.w * B.r33)
    };

    return Result;
}
m4
ProjectionMatrixOpenGl(real32 FOV, real32 AspectRatio,real32 n, real32 f)
{
    real32 TanHalfFOV = (real32)tan(FOV / 2.0f);

    m4 m = {};
    m._RC[0][0] = 1.0f / (AspectRatio * TanHalfFOV);
    m._RC[1][1] = 1.0f / (AspectRatio * TanHalfFOV);
    m._RC[2][2] = -(f + n) / (f - n);
    m._RC[3][2] = -1.0f;
    m._RC[2][3] = -(2.0f * f * n) / (f - n);

    return m;
}
m4
Projection(real32 FOV,real32 AspectRatio, real32 n, real32 f)
{
    m4 m = {};
#if 0
    // https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/opengl-perspective-projection-matrix
    real32 b,l,r;
    // tan(FOV*0.5) = op/ad = (height) / n => height = tan(FOV*0.5)*n
    real32 t = (real32)(tan(FOV * 0.5f)*(real64)n);
    b = -t;
    r = t * AspectRatio;
    l = -r;


    // Psz = (0*Px + 0*Py + A*Pz + B*Pw) / (-Pz)
    // Psz = (A*Pz + B*Pw) / -Pz
#endif

    real32 HalfTanFOV = (real32)(tan(FOV * 0.5f));
    real32 A = -(f + n) / (f - n);
    real32 B = (-2*f*n) / (f - n);

    m.c00 = 1.0f / (HalfTanFOV * AspectRatio); // (2 * n) / (r - l);
    // m.c20 = 0; //(r + l) / (r - l);
    m.c11 = 1.0f / (HalfTanFOV); // (2 * n) / (t - b);
    // m.c21 = 0; //(t + b) / (t - b);
    m.c22 = A;
    m.c32 = B;
    m.c23 = -1; // -Pz
    //m.c33 = 0;  // w

    return m;
} 

m4
Translate(m4 A, v3 B)
{
    A.c30 = B.x;
    A.c31 = B.y;
    A.c32 = B.z;

    return A;
}

inline real32 
ToRadians(real32 Degrees)
{
    //real32 Result = Degree * 2 * PI * (1.0f / 360.0f);
    real32 Result = Degrees * (real32)PI * (1.0f / 180.0f);
    return Result; 
}

m4
operator *(m4 A, m4 B)
{
    m4 Result = {};
    for (uint32 r = 0;
                r < 4;
                ++r)
    {
        for (uint32 c = 0;
                    c < 4;
                    ++c)
        {
            Result._RC[r][c] = A._RC[r][0] * B._RC[0][c] +
                               A._RC[r][1] * B._RC[1][c] +
                               A._RC[r][2] * B._RC[2][c] +
                               A._RC[r][3] * B._RC[3][c];
        }
    }

    return Result;
}


#define MATH_H
#endif
