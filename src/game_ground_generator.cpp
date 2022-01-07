#include "game.h"

v3
FlatHexCorner(v3 Center, r32 Size, u32 i)
{
    r32 Angle = 60.0f * r32(i);
    r32 InRad = ToRadians(Angle);
    v3 P = V3(Center.x + Size * cosf(InRad),
           0,
           Center.y + Size * sinf(InRad));
    return P;
}

mesh
CreateHexaGroundMesh(memory_arena * Arena)
{
    u32 TotalVertices = 18;
    vertex_point * Vertices = PushArray(Arena, TotalVertices, vertex_point);

    v3 Center = V3(0,0,0);
    v3 VUp = V3(0,1.0f,0);
    v4 Yellow = V4(1.0f,1.0f,0,1.0f);
    r32 Size = 1.0f;
    u32 CountVertices = 0;

    for (u32 HexCornerIndex = 0;
            HexCornerIndex < 6;
            ++HexCornerIndex)
    {
        Vertices[CountVertices].P = Center;
        Vertices[CountVertices].N = VUp;
        Vertices[CountVertices].Color = Yellow;
        ++CountVertices;

        Vertices[CountVertices].P = FlatHexCorner(Center, Size, HexCornerIndex);
        Vertices[CountVertices].N = VUp;
        Vertices[CountVertices].Color = Yellow;
        ++CountVertices;

        Vertices[CountVertices].P = FlatHexCorner(Center, Size, (HexCornerIndex + 1) % 6);
        Vertices[CountVertices].N = VUp;
        Vertices[CountVertices].Color = Yellow;
        ++CountVertices;
    }

    u32 BaseOffset = Arena->CurrentSize;

    mesh Mesh = {};

    Mesh.Vertices       = Vertices;                             // vertex_point * Vertices;
    Mesh.VertexSize     = TotalVertices * sizeof(vertex_point); // u32 VertexSize;
    Mesh.Indices        = 0;                                    // Typedef * Indices;
    Mesh.IndicesSize    = 0;                                    // u32 IndicesSize;
    Mesh.OffsetVertices = BaseOffset;                           // u32 OffsetVertices;
    Mesh.OffsetIndices  = 0;                                    // u32 OffsetIndices;
    Mesh.Loaded         = true;                                 // b32 Loaded;
    Mesh.LoadInProcess  = false;                                // b32 LoadInProcess;

    RenderPushVertexData(Mesh.Vertices,Mesh.VertexSize, BaseOffset);
    PushMeshSize(Arena, Mesh.VertexSize, 1);

    return Mesh;
}

