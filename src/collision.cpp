#include "game.h"
#include "math.h"


inline bool32 
TestSphereSphere(sphere a, sphere b)
{
    v3 d = a.c - b.c;
    real32 LengthSquare = LengthSqr(d);
    real32 SumR = a.r + b.r;

    bool32 Result = (LengthSquare <= (SumR * SumR));

    return Result;
}

void
FindPointsFarthestApart(vertex_point * Vertices, uint32 VertexCount, uint32 * Min, uint32 * Max)
{
    uint32 MinX = 0, MaxX = 0;
    uint32 MinZ = 0, MaxZ = 0;
    uint32 MinY = 0, MaxY = 0;

    for (uint32 VertexIndex = 0;
                VertexIndex < VertexCount;
                ++VertexIndex)
    {
        if (Vertices[VertexIndex].P.x < Vertices[MinX].P.x) MinX = VertexIndex;
        if (Vertices[VertexIndex].P.x > Vertices[MaxX].P.x) MaxX = VertexIndex;

        if (Vertices[VertexIndex].P.y < Vertices[MinY].P.y) MinY = VertexIndex;
        if (Vertices[VertexIndex].P.y > Vertices[MaxY].P.y) MaxY = VertexIndex;

        if (Vertices[VertexIndex].P.z < Vertices[MinZ].P.z) MinZ = VertexIndex;
        if (Vertices[VertexIndex].P.z > Vertices[MaxZ].P.z) MaxZ = VertexIndex;
    }

    real32 LengthSquareX = LengthSqr(Vertices[MaxX].P - Vertices[MinX].P);
    real32 LengthSquareY = LengthSqr(Vertices[MaxY].P - Vertices[MinY].P);
    real32 LengthSquareZ = LengthSqr(Vertices[MaxZ].P - Vertices[MinZ].P);

    *Min = MinX;
    *Max = MaxX;

    if (LengthSquareY > LengthSquareX && LengthSquareY > LengthSquareZ)
    {
        *Min = MinY;
        *Max = MaxY;
    }
    else if (LengthSquareZ > LengthSquareX && LengthSquareZ > LengthSquareY)
    {
        *Min = MinZ;
        *Max = MaxZ;
    }
}

void
SphereFromDistantPoints(sphere * S, vertex_point * Vertices, uint32 VertexCount)
{
    uint32 Min,Max;
    FindPointsFarthestApart(Vertices, VertexCount, &Min, &Max);
    S->c = (Vertices[Min].P + Vertices[Max].P) * 0.5f;
    S->r = Length(Vertices[Max].P - S->c);
}

void
SphereOfSphereAndPoint(sphere * S, v3 * P)
{
    v3 CenterToP = (*P) - S->c;
    real32 LengthSquare = LengthSqr(CenterToP);
    if (LengthSquare > (S->r * S->r))
    {
        real32 Dist = sqrtf(LengthSquare);
        real32 NewR = (S->r + Dist) * 0.5f;
        real32 k = (NewR - S->r) / Dist;
        S->r = NewR;
        S->c += CenterToP * k;
    }
}

void
RitterSphere(sphere * S, vertex_point * Vertices, uint32 VertexCount)
{
    SphereFromDistantPoints(S,Vertices,VertexCount);
    for (uint32 VertexIndex = 0;
            VertexIndex < VertexCount;
            ++VertexIndex)
    {
        SphereOfSphereAndPoint(S, &Vertices[VertexIndex].P);
    }
}

