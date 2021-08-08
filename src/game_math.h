#ifndef GAME_MATH_H

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

inline v2
V2(real32 x, real32 y)
{
    v2 Result = { x, y};
    return Result;
}
inline real32
LengthSqr(v2 A)
{
    real32 Result = A.x*A.x + A.y*A.y;
    return Result;
}
inline real32
Length(v2 A)
{
    real32 Result = (real32)sqrtf(LengthSqr(A));
    return Result;
}

inline v2
operator *(v2 A, real32 B)
{
    v2 Result = {A.x * B, A.y * B};
    return Result;
}

inline v2
operator /(v2 A, real32 B)
{
    real32 OneOver = 1.0f / B;
    v2 Result = A * OneOver;
    return Result;
}

inline v2
operator -(v2 A, v2 B)
{
    v2 Result = {A.x - B.x, A.y - B.y};
    return Result;
}

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


inline v3
V3(real32 x, real32 y, real32 z)
{
    v3 Result = { x, y, z};
    return Result;
}

inline v3
V3(real32 w)
{
    v3 Result = { w, w, w};
    return Result;
}

inline v3
V3(v2 A, real32 z)
{
    v3 Result = { A.x, A.y, z};
    return Result;
}

inline v3
operator -(v3 A)
{
    v3 Result = { -A.x, -A.y, -A.z};
    return Result;
}

inline v3
Cross(v3 A, v3 B)
{
    v3 Result = {
        A.y*B.z - A.z*B.y,
        A.z*B.x - A.x*B.z,
        A.x*B.y - A.y*B.x
    };


    return Result;
}

inline v3
operator -(v3 A, v3 B)
{
    v3 Result = { A.x - B.x, A.y - B.y, A.z - B.z };

    return Result;
}
inline v3
operator +(v3 A, v3 B)
{
    v3 Result = { A.x + B.x, A.y + B.y, A.z + B.z };

    return Result;
}

inline v3
operator +=(v3 &A, v3 B)
{
    A = A + B;

    return A;
}

inline v3
operator *(v3 A, real32 B)
{
    v3 Result = {A.x * B, A.y * B, A.z * B};

    return Result;
}


inline v3
operator *=(v3 &A, real32 B)
{
    A = A*B;

    return A;
}

inline v3
operator /(v3 A, real32 B)
{
    v3 Result = A * (1.0f / B);

    return Result;
}


inline v3
VectorMultiply(v3 A, v3 B)
{
    v3 R = {
        A.x * B.x,
        A.y * B.y,
        A.z * B.z
    };

    return R;
}

inline real32
Inner(v3 A, v3 B)
{
    real32 Result = (A.x * B.x + A.y * B.y + A.z * B.z);
    return Result;
}
inline real32
LengthSqr(v3 A)
{
    real32 Result = Inner(A, A);
    return Result;
}
inline real32
Length(v3 A)
{
    real32 Result = sqrtf(LengthSqr(A));
    return Result;
}

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
    const real32& operator[](int index) const
    {
        return _V[index];
    }
    real32& operator[](int index)
    {
        return _V[index];
    }
};

inline v4
V4(real32 x, real32 y, real32 z, real32 w)
{
    v4 Result = { x, y, z, w};
    return Result;
}
inline v4
V4(v3 v, real32 w)
{
    v4 Result = { v.x, v.y, v.z, w};
    return Result;
}

inline real32
Inner(v4 A, v4 B)
{
    real32 Result = (A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w);
    return Result;
}

inline v3
operator *(real32 R, v3 V)
{
    v3 Result = { V.x * R, V.y * R, V.z * R };
    return Result;
}


inline real32 
ToRadians(real32 Degrees)
{
    //real32 Result = Degree * 2 * PI * (1.0f / 360.0f);
    real32 Result = Degrees * (real32)PI * (1.0f / 180.0f);
    return Result; 
}


struct m4
{
    v4 Columns[4];
    const v4& operator[](int index) const
    {
        return Columns[index];
    }
    v4& operator[](int index)
    {
        return Columns[index];
    }

};

inline m4
M4()
{
    m4 Result;

    Result.Columns[0] = V4(1,0,0,0);
    Result.Columns[1] = V4(0,1,0,0);
    Result.Columns[2] = V4(0,0,1,0);
    Result.Columns[3] = V4(0,0,0,1);

    return Result;
}
inline m4
M4(v3 V)
{
    m4 Result;

    Result.Columns[0] = V4(V.x,0,0,0);
    Result.Columns[1] = V4(0,V.y,0,0);
    Result.Columns[2] = V4(0,0,V.z,0);
    Result.Columns[3] = V4(0,0,0,1);

    return Result;
}

inline v4
operator +(const v4& A, const v4& B)
{
    v4 R = { A.x + B.x, A.y + B.y, A.z + B.z, A.w + B.w };

    return R;
}

inline m4
operator +(const m4& A, const m4& B)
{
    m4 R;
    R[0] = A[0] + B[0];
    R[1] = A[1] + B[1];
    R[2] = A[2] + B[2];
    R[3] = A[3] + B[3];

    return R;
}

// pre-multiply
inline v4
operator *(const m4& M, const v4& V)
{
    v4 R = V4( 
        M[0].x * V.x + M[1].x * V.y + M[2].x * V.z + M[3].x * V.w,
        M[0].y * V.x + M[1].y * V.y + M[2].y * V.z + M[3].y * V.w,
        M[0].z * V.x + M[1].z * V.y + M[2].z * V.z + M[3].z * V.w,
        M[0].w * V.x + M[1].w * V.y + M[2].w * V.z + M[3].w * V.w
    );
    return R;
}
// post-multiply
inline v4
operator *(const v4& V, const m4& M)
{
    v4 R = V4( 
            Inner(M[0],V),
            Inner(M[1],V),
            Inner(M[2],V),
            Inner(M[3],V)
    );

    return R;
}
inline m4
operator *(const m4 &M1, const m4 &M2)
{
    m4 MR;

    MR.Columns[0] = M1 * M2[0];
    MR.Columns[1] = M1 * M2[1];
    MR.Columns[2] = M1 * M2[2];
    MR.Columns[3] = M1 * M2[3];

    return MR;
}

inline v3
Normalize(v3 A)
{
    v3 R = A / Length(A);
    return R;
}


#define GAME_MATH_H
#endif
