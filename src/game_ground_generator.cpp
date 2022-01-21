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

mesh_group
CreateHexaGroundMesh(memory_arena * Arena)
{
    u32 TotalVertices = 18;

    mesh_group MeshGroup = {};
    mesh * Mesh = PushArray(Arena, 1,mesh);

    vertex_point * Vertices = PushArray(Arena, TotalVertices, vertex_point);

    v3 CenterOrigin = V3(0,0,0);
    v3 VUp = V3(0,1.0f,0);
    v4 Yellow = V4(1.0f,1.0f,0,1.0f);
    v4 Red = V4(1.0f,0.0f,0,1.0f);
    r32 Size = 1.0f;
    u32 CountVertices = 0;

    r32 RandomDecimal[] = 
    { 
        0.31f,0.39f,0.01f,0.11f,0.66f,0.27f,0.32f,0.56f,0.19f,0.36f,0.12f,0.01f,0.76f,0.37f,0.70f,0.88f,0.16f,0.54f,0.54f,0.81f,0.13f,0.67f,0.90f,0.98f,0.32f,0.84f,0.99f,0.14f,0.02f,0.97f,0.77f,0.93f,0.63f,0.70f,0.78f,0.17f,1.00f,0.19f,0.50f,0.67f,0.47f,0.45f,0.64f,0.06f,0.38f,0.10f,0.68f,0.45f,0.00f,0.63f,0.40f,0.32f,0.72f,0.41f,0.85f,0.15f,0.45f,0.23f,0.34f,0.15f,0.79f,0.51f,0.65f,0.76f,0.84f,0.59f,0.85f,0.35f,0.11f,0.73f,0.62f,0.54f,0.64f,0.14f,0.86f,0.16f,0.83f,0.44f,0.02f,0.73f,0.19f,0.62f,0.96f,0.08f,0.41f,0.17f,0.27f,0.78f,0.18f,0.49f,0.31f,0.35f,0.93f,0.75f,0.21f,0.25f,0.92f,0.05f,0.27f,0.08f,0.16f,0.36f,0.13f,0.46f,0.03f,0.56f,0.10f,0.34f,0.15f,0.69f,0.27f,0.62f,0.60f,0.79f,0.49f,0.05f,0.98f,0.06f,0.16f,0.94f,0.80f,0.91f,0.77f,0.36f,0.58f,0.80f,0.00f,0.57f,0.90f,0.99f,0.39f,0.60f,0.03f,0.86f,0.49f,0.27f,0.25f,0.51f,0.08f,0.31f,0.75f,0.62f,0.48f,0.64f,0.56f,0.12f,0.63f,0.88f,0.32f,0.32f,0.72f,0.01f,0.19f,0.05f,0.58f,0.66f,0.49f,0.02f,0.73f,0.50f,0.83f,0.59f,0.95f,0.83f,0.17f,0.82f,0.29f,0.80f,0.04f,0.47f,0.34f,0.01f,0.97f,0.81f,0.09f,0.75f,0.15f,0.09f,0.38f,0.24f,0.16f,0.83f,0.05f,0.27f,0.37f,0.36f,0.66f,0.42f,0.20f,0.41f,0.70f,0.55f,0.66f,0.95f,0.87f,0.58f,0.94f,0.05f,0.99f,0.38f,0.21f,1.00f,0.75f,0.86f,0.96f,0.67f,0.52f,0.05f,0.85f,0.30f,0.09f,0.23f,0.52f,0.76f,0.91f,0.89f,0.49f,0.07f,0.13f,0.92f,0.63f,0.07f,0.46f,0.18f,0.38f,0.69f,0.90f,0.37f,0.59f,0.21f,0.53f,0.57f,0.47f,0.79f,0.33f,0.31f,0.82f,0.26f,0.37f,0.53f,0.71f,0.77f,0.92f,0.27f,0.35f,0.07f,0.79f,0.06f,0.52f,0.26f,0.44f,0.14f,0.36f,0.62f,0.96f,0.87f,0.33f,0.01f,0.94f,0.41f,0.03f,0.88f,0.30f,0.40f,0.44f,0.74f,0.92f,0.33f,0.66f,0.91f,0.75f,0.08f,0.39f,0.00f,0.88f,0.76f,0.24f,0.72f,0.67f,0.07f,0.58f,0.67f,0.54f,0.52f,0.56f,0.84f,0.33f,0.40f,0.15f,0.07f,0.39f,0.46f,0.23f,0.50f,0.54f,0.38f,0.93f,0.46f,0.58f,0.45f
    };
    v3 Colors = {};

    for (u32 HexIndex = 0;
                HexIndex < 10;
                ++HexIndex)
    {
        r32 x = (((u32)RandomDecimal[HexIndex] * 100) % 10) * Size;
        r32 z = (((u32)RandomDecimal[HexIndex] * 32) % 10) * Size;
        v4 Color = Yellow;
        v3 Center = CenterOrigin + V3(x,0,z);
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
    }

    u32 BaseOffset = Arena->CurrentSize;

    Mesh->Vertices       = Vertices;                             // vertex_point * Vertices;
    Mesh->VertexSize     = TotalVertices * sizeof(vertex_point); // u32 VertexSize;
    Mesh->Indices        = 0;                                    // Typedef * Indices;
    Mesh->IndicesSize    = 0;                                    // u32 IndicesSize;
    Mesh->OffsetVertices = BaseOffset;                           // u32 OffsetVertices;
    Mesh->OffsetIndices  = 0;                                    // u32 OffsetIndices;

    MeshGroup.Loaded         = true;                                 // b32 Loaded;
    MeshGroup.LoadInProcess  = false;                                // b32 LoadInProcess;

    RenderPushVertexData(Mesh->Vertices,Mesh->VertexSize, BaseOffset);
    PushMeshSize(Arena, Mesh->VertexSize, 1);

    return MeshGroup;
}

