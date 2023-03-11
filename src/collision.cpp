#include "collision.h"
#include <float.h>
#include "game_math.h"

#define EPSILON 0.00001f

// Compute indices to the two most separated points of the (up to) six points
// defining the AABB encompassing the point set. Return these as min and max.
void MostSeparatedPointsOnAABB(int &min, int &max, vertex_point * pt, int numPts)
{
    // First find most extreme points along principal axes
    int minx = 0, maxx = 0, miny = 0, maxy = 0, minz = 0, maxz = 0;
    for (int i = 1; i < numPts; i++) {
        if (pt[i].P.x < pt[minx].P.x) minx = i;
        if (pt[i].P.x > pt[maxx].P.x) maxx = i;
        if (pt[i].P.y < pt[miny].P.y) miny = i;
        if (pt[i].P.y > pt[maxy].P.y) maxy = i;
        if (pt[i].P.z < pt[minz].P.z) minz = i;
        if (pt[i].P.z > pt[maxz].P.z) maxz = i;
    }
    // Compute the squared distances for the three pairs of points
    r32 dist2x = Inner(pt[maxx].P - pt[minx].P, pt[maxx].P - pt[minx].P);
    r32 dist2y = Inner(pt[maxy].P - pt[miny].P, pt[maxy].P - pt[miny].P);
    r32 dist2z = Inner(pt[maxz].P - pt[minz].P, pt[maxz].P - pt[minz].P);
    // Pick the pair (min,max) of points most distant
    min = minx;
    max = maxx;
    if (dist2y > dist2x && dist2y > dist2z) {
        max = maxy;
        min = miny;
    }
    if (dist2z > dist2x && dist2z > dist2y) {
        max = maxz;
        min = minz;
    }
}
void SphereFromDistantPoints(sphere * s, vertex_point * pt, u32 VertexCount)
{
    // Find the most separated point pair defining the encompassing AABB
    int min, max;
    MostSeparatedPointsOnAABB(min, max, pt, VertexCount);
    // Set up sphere to just encompass these two points
    s->c = (pt[min].P + pt[max].P) * 0.5f;
    s->r = Inner(pt[max].P - s->c, pt[max].P - s->c);
    s->r = sqrtf(s->r);
}
#if 0
void
SphereFromDistantPoints(sphere * S, vertex_point * Vertices, u32 VertexCount)
{
    u32 Min,Max;
    FindPointsFarthestApart(Vertices, VertexCount, &Min, &Max);
    S->c = (Vertices[Min].P + Vertices[Max].P) * 0.5f;
    S->r = Length(Vertices[Max].P - S->c);
}
#endif

inline b32 
TestSphereSphere(sphere a, sphere b)
{
    v3 d = a.c - b.c;
    r32 LengthSquare = LengthSqr(d);
    r32 SumR = a.r + b.r;

    b32 Result = (LengthSquare <= (SumR * SumR));

    return Result;
}

void
FindPointsFarthestApart(vertex_point * Vertices, u32 VertexCount, u32 * Min, u32 * Max)
{
    u32 MinX = 0, MaxX = 0;
    u32 MinZ = 0, MaxZ = 0;
    u32 MinY = 0, MaxY = 0;

    for (u32 VertexIndex = 0;
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

    r32 LengthSquareX = LengthSqr(Vertices[MaxX].P - Vertices[MinX].P);
    r32 LengthSquareY = LengthSqr(Vertices[MaxY].P - Vertices[MinY].P);
    r32 LengthSquareZ = LengthSqr(Vertices[MaxZ].P - Vertices[MinZ].P);

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
SphereOfSphereAndPoint(sphere * S, v3 * P)
{
    v3 CenterToP = (*P) - S->c;
    r32 LengthSquare = LengthSqr(CenterToP);
    if (LengthSquare > (S->r * S->r))
    {
        r32 Dist = sqrtf(LengthSquare);
        r32 NewR = (S->r + Dist) * 0.5f;
        r32 k = (NewR - S->r) / Dist;
        S->r = NewR;
        S->c += CenterToP * k;
    }
}

void
RitterSphere(sphere * S, vertex_point * Vertices, u32 VertexCount)
{
    SphereFromDistantPoints(S,Vertices,VertexCount);
    for (u32 VertexIndex = 0;
            VertexIndex < VertexCount;
            ++VertexIndex)
    {
        SphereOfSphereAndPoint(S, &Vertices[VertexIndex].P);
    }
}

b32
CollisionTestAABBAABB(AABB a, AABB b)
{
    if (fabs(a.c.x - b.c.x) > (a.r.x + b.r.x)) return false;
    if (fabs(a.c.y - b.c.y) > (a.r.y + b.r.y)) return false;
    if (fabs(a.c.z - b.c.z) > (a.r.z + b.r.z)) return false;
    return true;
}

void 
ExtremePointsAlongDirection(v3 dir, vertex_point * pt, int n, int *imin, int *imax)
{
    r32 minproj = FLT_MAX, maxproj = -FLT_MAX;
    for (int i = 0; i < n; i++) {
        // Project vector from origin to point onto direction vector
        r32 proj = Inner(pt[i].P, dir);
        // Keep track of least distant point along direction vector
        if (proj < minproj) {
            minproj = proj;
            *imin = i;
        }
        // Keep track of most distant point along direction vector
        if (proj > maxproj) {
            maxproj = proj;
            *imax = i;
        }
    }
}

b32
TestCollisionSpheres(sphere s0, sphere s1, v3 dP0, v3 dP1, r32 & t)
{
    v3 s = s0.c - s1.c;
    v3 v = dP0 - dP1;
    r32 dist = LengthSqr(s);
    r32 r = s0.r + s1.r;
    r32 c = dist - SQR(r);

    if (c < 0.0f) 
    {
        // overlaps
        t = 0.0f;
        return true;
    }

    r32 a = LengthSqr(v);

    if (a < EPSILON)
    {
        // not moving relative
        return false;
    }
    
    r32 b = Inner(v,s);

    if (b >= 0.0f)
    {
        // moving opp direc
        return false;
    }

    r32 d = SQR(b) - (a * c);

    if (d < 0.0f)
    {
        // no real value root
        return false;
    }

    t = (-b - sqrtf(d)) / a;

    return true;
}
