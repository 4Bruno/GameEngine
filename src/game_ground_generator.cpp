#include "game.h"
#include "noise_perlin.cpp"
#include "game_math.h"
#include "game_ground_generator.h"

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

void
CreateGroundAtPosition(game_state * GameState, world_pos WorldP)
{
#if 0
    u32 TotalTiles = 10;

    r32 RandomDecimal[] = 
    { 
        0.31f,0.39f,0.01f,0.11f,0.66f,0.27f,0.32f,0.56f,0.19f,0.36f,0.12f,0.01f,0.76f,0.37f,0.70f,0.88f,0.16f,0.54f,0.54f,0.81f,0.13f,0.67f,0.90f,0.98f,0.32f,0.84f,0.99f,0.14f,0.02f,0.97f,0.77f,0.93f,0.63f,0.70f,0.78f,0.17f,1.00f,0.19f,0.50f,0.67f,0.47f,0.45f,0.64f,0.06f,0.38f,0.10f,0.68f,0.45f,0.00f,0.63f,0.40f,0.32f,0.72f,0.41f,0.85f,0.15f,0.45f,0.23f,0.34f,0.15f,0.79f,0.51f,0.65f,0.76f,0.84f,0.59f,0.85f,0.35f,0.11f,0.73f,0.62f,0.54f,0.64f,0.14f,0.86f,0.16f,0.83f,0.44f,0.02f,0.73f,0.19f,0.62f,0.96f,0.08f,0.41f,0.17f,0.27f,0.78f,0.18f,0.49f,0.31f,0.35f,0.93f,0.75f,0.21f,0.25f,0.92f,0.05f,0.27f,0.08f,0.16f,0.36f,0.13f,0.46f,0.03f,0.56f,0.10f,0.34f,0.15f,0.69f,0.27f,0.62f,0.60f,0.79f,0.49f,0.05f,0.98f,0.06f,0.16f,0.94f,0.80f,0.91f,0.77f,0.36f,0.58f,0.80f,0.00f,0.57f,0.90f,0.99f,0.39f,0.60f,0.03f,0.86f,0.49f,0.27f,0.25f,0.51f,0.08f,0.31f,0.75f,0.62f,0.48f,0.64f,0.56f,0.12f,0.63f,0.88f,0.32f,0.32f,0.72f,0.01f,0.19f,0.05f,0.58f,0.66f,0.49f,0.02f,0.73f,0.50f,0.83f,0.59f,0.95f,0.83f,0.17f,0.82f,0.29f,0.80f,0.04f,0.47f,0.34f,0.01f,0.97f,0.81f,0.09f,0.75f,0.15f,0.09f,0.38f,0.24f,0.16f,0.83f,0.05f,0.27f,0.37f,0.36f,0.66f,0.42f,0.20f,0.41f,0.70f,0.55f,0.66f,0.95f,0.87f,0.58f,0.94f,0.05f,0.99f,0.38f,0.21f,1.00f,0.75f,0.86f,0.96f,0.67f,0.52f,0.05f,0.85f,0.30f,0.09f,0.23f,0.52f,0.76f,0.91f,0.89f,0.49f,0.07f,0.13f,0.92f,0.63f,0.07f,0.46f,0.18f,0.38f,0.69f,0.90f,0.37f,0.59f,0.21f,0.53f,0.57f,0.47f,0.79f,0.33f,0.31f,0.82f,0.26f,0.37f,0.53f,0.71f,0.77f,0.92f,0.27f,0.35f,0.07f,0.79f,0.06f,0.52f,0.26f,0.44f,0.14f,0.36f,0.62f,0.96f,0.87f,0.33f,0.01f,0.94f,0.41f,0.03f,0.88f,0.30f,0.40f,0.44f,0.74f,0.92f,0.33f,0.66f,0.91f,0.75f,0.08f,0.39f,0.00f,0.88f,0.76f,0.24f,0.72f,0.67f,0.07f,0.58f,0.67f,0.54f,0.52f,0.56f,0.84f,0.33f,0.40f,0.15f,0.07f,0.39f,0.46f,0.23f,0.50f,0.54f,0.38f,0.93f,0.46f,0.58f,0.45f
    };

    for (u32 HexIndex = 0;
                HexIndex < TotalTiles;
                ++HexIndex)
    {
        r32 x = ((u32)(RandomDecimal[HexIndex] * 100.0f) % 10) * 1.0f;
        r32 z = ((u32)(RandomDecimal[HexIndex] * 32.0f) % 10) * 1.0f;
        v3 GroundOffsetP = V3(x,0,z);
        //v3 GroundOffsetP = V3(0,0,0);

        entity * Entity = AddEntity(&GameState->World, WorldP);
        EntityAddTranslation(Entity,0,GroundOffsetP, V3(1.0f),0.0f);
        Entity->Color = V3(1.0f,0,0);
        Entity->IsGround = true;
    }
    v3 GroundOffsetP = V3(x,0,z);
    //v3 GroundOffsetP = V3(0,0,0);

    entity * Entity = AddEntity(&GameState->World, WorldP);
    EntityAddTranslation(Entity,0,GroundOffsetP, V3(1.0f),0.0f);
    Entity->Color = V3(1.0f,0,0);
    Entity->IsGround = true;
#else

#endif
}

void
CreateHexaGroundMesh(mesh_group * MeshGroup, memory_arena * TempArena, memory_arena * VertexBufferArena, u32 MaxGroundByteSize)
{
    u32 TotalVertices = 18;

    MeshGroup->TotalMeshObjects = 1;
    mesh * Mesh = MeshGroup->Meshes;


    u32 OriginalMeshSize = TotalVertices * sizeof(vertex_point);
    u32 Align = RenderGetVertexMemAlign() - 1;
    u32 MeshSize = (OriginalMeshSize + Align) & ~Align;

    Assert(MeshSize >= OriginalMeshSize);

    // Ground is always first mesh in buffer
    u32 VertexBufferBeginOffset = 0;

    Mesh->OffsetVertices = VertexBufferBeginOffset;
    Mesh->VertexSize     = OriginalMeshSize;
    Mesh->IndicesSize    = 0; // u32 IndicesSize;
    Mesh->OffsetIndices  = 0; // u32 OffsetIndices;
    
    BeginTempArena(TempArena,1);

    vertex_point * Vertices = (vertex_point *)PushSize(TempArena, MeshSize);

    // create hex vertices list
    v3 Center = V3(0,0,0);
    v3 VUp = V3(0,1.0f,0);
    v4 Yellow = V4(1.0f,1.0f,0,1.0f);
    v4 Red = V4(1.0f,0.0f,0,1.0f);
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

    RenderPushVertexData(Vertices,MeshSize, VertexBufferBeginOffset); 

    EndTempArena(TempArena,1);

    MeshGroup->Loaded        = true;  // b32 Loaded;
    MeshGroup->LoadInProcess = false; // b32 LoadInProcess;
}

b32
InBetweenExcl(u32 Value,u32 Start,u32 End)
{
    b32 Result = (Value > Start) &&
                 (Value < End);

    return Result;
}

THREAD_WORK_HANDLER(LoadGround)
{
    async_load_ground * WorkData = (async_load_ground *)Data;
    Assert(WorkData->ThreadArena);
    
    world * World                    = WorkData->World;
    mesh_group * MeshGroup           = WorkData->MeshGroup;
    memory_arena * TempArena         = &WorkData->ThreadArena->Arena;
    world_pos WorldP                 = WorkData->WorldP;

    // Define max vertices
    i32 TotalXTiles = WorkData->TotalXTiles;
    i32 TotalZTiles = WorkData->TotalZTiles;

    mesh * Mesh = MeshGroup->Meshes;
    u32 VertexBufferBeginOffset = Mesh->OffsetVertices;

    //u32 TotalTriangles = (TotalXTiles - 1) * (TotalZTiles - 1) * 2;
    u32 TotalTriangles = (TotalXTiles - 1) * (TotalZTiles - 1) * 2;
    u32 TotalVertices = TotalTriangles * 3;

    u32 OriginalMeshSize = TotalVertices * sizeof(vertex_point);
    u32 Align = RenderGetVertexMemAlign() - 1;
    u32 MeshSize = (OriginalMeshSize + Align) & ~Align;
    Assert(MeshSize >= OriginalMeshSize);

    u32 TotalVertexPoints = TotalXTiles * TotalZTiles;
    v3 * VertexPoints = PushArray(TempArena,TotalVertexPoints,v3);

    // Mapping cells coordinates to range [1,n]
    // the gradient between buckets is based on current worldP offset
    r32 Density = 1.0f;
    r32 DensityMultiply = (1.0f / Density);

    v3 GroundP = V3((r32)WorldP.x * DensityMultiply,(r32)WorldP.y * DensityMultiply,(r32)WorldP.z * DensityMultiply);
    //v3 GroundP = V3((r32)WorldP.x ,(r32)WorldP.y ,(r32)WorldP.z );

    u32 VertexIndex = 0;
    r32 MaxX = 0;
    r32 MaxZ = 0;
    r32 MinX = 0;
    r32 MinZ = 0;

    r32 MinY = 0;
    r32 MaxY = 0;

    r32 OneOverTotalXTiles = (1.0f / (TotalXTiles - 1));
    r32 OneOverTotalZTiles = (1.0f / (TotalZTiles - 1));

    r32 MaxPx = 1.0f;
    r32 MaxPz = 1.0f;

    // TODO: how?
    r32 MaxPy = maxval(MaxPx, MaxPz) + (r32)fabs((r32)WorldP.y - 1) * DensityMultiply;
    //v3 OneOverMaxRangeValues = V3(1.0f / MaxPx, 1.0f / MaxPy, 1.0f / MaxPz);
    v3 OneOverMaxRangeValues = V3(1.0f / MaxPx, 1.0f / MaxPy, 1.0f / MaxPz);

    for (i32 TileX = 0;
                TileX < TotalXTiles;
                ++TileX)
    {
        r32 Px = (r32)(WorldP.x + TileX * OneOverTotalXTiles) * DensityMultiply;
        for (i32 TileZ = 0;
                TileZ < TotalZTiles;
                ++TileZ)
        {
            r32 Pz = (r32)(WorldP.z + TileZ * OneOverTotalZTiles) * DensityMultiply;
            r32 Py = perlin(Px, Pz);
            //r32 Py = 0.5f;
            v3 Vertex = V3(Px,Py,Pz) - GroundP;
            //Vertex = VectorMultiply(Vertex,OneOverMaxRangeValues);
            *(VertexPoints + VertexIndex) = Vertex;
            Logn("Vertex:" STRP,FP(Vertex));
            ++VertexIndex;
        }
    }
    //Logn("Max x: %10f Max z: %10f Max y: %10f",MaxX, MaxZ, MaxY);
    //Logn("Min x: %10f Min z: %10f Min y: %10f",MinX, MinZ, MinY);

    v3 VUp = V3(0,1.0f,0);
    v4 Yellow = V4(1.0f,1.0f,0,1.0f);
    vertex_point * Vertices = (vertex_point *)PushSize(TempArena, MeshSize);
    u32 CountVertices = 0;
    u32 Stride = TotalZTiles;

    for (i32 TileX = 0;
                TileX < (TotalXTiles - 1);
                ++TileX)
    {
        // last Z tile is done in the prior row
        for (i32 TileZ = 0;
                TileZ < TotalZTiles;
                ++TileZ)
        {
            // right and up
            u32 OffsetX = 1;
            u32 OffsetZ = Stride;

            if (TileZ == (TotalZTiles - 1))
            {
                OffsetX = Stride;
                OffsetZ = Stride - 1;
            }

            v3 * va = VertexPoints + (Stride * TileX) + TileZ;
            v3 * vb = va + OffsetX;
            v3 * vc = va + OffsetZ;

            Vertices[CountVertices].P = *va;
            Vertices[CountVertices].N = VUp;
            Vertices[CountVertices].Color = Yellow;
            ++CountVertices;

            Vertices[CountVertices].P = *vb;
            Vertices[CountVertices].N = VUp;
            Vertices[CountVertices].Color = Yellow;
            ++CountVertices;

            Vertices[CountVertices].P = *vc;
            Vertices[CountVertices].N = VUp;
            Vertices[CountVertices].Color = Yellow;
            ++CountVertices;

            if (InBetweenExcl(TileZ,0,Stride - 1))
            {
                OffsetX = Stride;
                OffsetZ = Stride - 1;

                vb = va + OffsetX;
                vc = va + OffsetZ;

                Vertices[CountVertices].P = *va;
                Vertices[CountVertices].N = VUp;
                Vertices[CountVertices].Color = Yellow;
                ++CountVertices;

                Vertices[CountVertices].P = *vb;
                Vertices[CountVertices].N = VUp;
                Vertices[CountVertices].Color = Yellow;
                ++CountVertices;

                Vertices[CountVertices].P = *vc;
                Vertices[CountVertices].N = VUp;
                Vertices[CountVertices].Color = Yellow;
                ++CountVertices;
            }
        }
    }


    RenderPushVertexData(Vertices,MeshSize, VertexBufferBeginOffset); 

    ThreadEndArena(WorkData->ThreadArena);

    COMPILER_DONOT_REORDER_BARRIER;
    MeshGroup->Loaded        = true;  // b32 Loaded;
    MeshGroup->LoadInProcess = false; // b32 LoadInProcess;
}

/*
 * Signals whether managed to get thread arena.
 * if it didn't, no work was done
 */
bool
TryLoadGround(game_memory * Memory,
              game_state * GameState,
              world_pos WorldP, mesh_group * MeshGroup, u32 TotalXTiles, u32 TotalZTiles)
{
    b32 LoadSuccess = false;

    thread_memory_arena * ThreadArena = GetThreadArena(GameState);
    if (IS_NOT_NULL(ThreadArena))
    {
        MeshGroup->LoadInProcess = true;

        memory_arena * Arena = ThreadBeginArena(ThreadArena);

        async_load_ground * Data = PushStruct(Arena,async_load_ground);

        MeshGroup->TotalMeshObjects = 1;

        Assert(IS_NOT_NULL(MeshGroup->Meshes)); // should be pre-allocated on world initialization

        Data->World       = &GameState->World; // world * World;
        Data->ThreadArena = ThreadArena;       // thread_memory_arena * ThreadArena;
        Data->MeshGroup   = MeshGroup;         // mesh_group * MeshGroup;
        Data->WorldP      = WorldP;            // RECORD WorldP;
        Data->TotalXTiles = TotalXTiles;       // i32 TotalXTiles;
        Data->TotalZTiles = TotalZTiles;       // i32 TotalZTiles;

#if 0
        u32 TotalTriangles = (TotalXTiles - 1) * (TotalZTiles - 1) * 2;
        u32 TotalVertices = TotalTriangles * 3;

        u32 OriginalMeshSize = TotalVertices * sizeof(vertex_point);
        u32 Align = RenderGetVertexMemAlign() - 1;
        u32 MeshSize = (OriginalMeshSize + Align) & ~Align;

        Assert(MeshSize > 0);
        Assert(MeshSize < MeshGroup->Meshes->VertexSize);
#endif
        Memory->AddWorkToWorkQueue(Memory->RenderWorkQueue , LoadGround,Data);

        LoadSuccess = true;
    }

    return LoadSuccess;
}

void
CreateGroundMeshPerlin(world * World, 
                       mesh_group * MeshGroup, 
                       memory_arena * TempArena, 
                       memory_arena * VertexBufferArena, 
                       u32 MaxGroundByteSize, 
                       world_pos WorldP)
{
    //Logn("Seed World P " STRWORLDP, FWORLDP(WorldP));

    // Define max vertices
    i32 TotalXTiles = 30;
    i32 TotalZTiles = 30;

    i32 XTiles = (TotalXTiles / 2);
    i32 ZTiles = (TotalZTiles / 2);

    MeshGroup->TotalMeshObjects = 1;
    mesh * Mesh = MeshGroup->Meshes;

    u32 TotalTriangles = (TotalXTiles - 1) * (TotalZTiles - 1) * 2;
    u32 TotalVertices = TotalTriangles * 3;

    u32 OriginalMeshSize = TotalVertices * sizeof(vertex_point);
    u32 Align = RenderGetVertexMemAlign() - 1;
    u32 MeshSize = (OriginalMeshSize + Align) & ~Align;

    Assert(MeshSize < MaxGroundByteSize);

    Assert(MeshSize >= OriginalMeshSize);

    u32 VertexBufferBeginOffset = 0;

    Mesh->OffsetVertices = VertexBufferBeginOffset;
    Mesh->VertexSize     = OriginalMeshSize;
    Mesh->IndicesSize    = 0; // u32 IndicesSize;
    Mesh->OffsetIndices  = 0; // u32 OffsetIndices;
    
    BeginTempArena(TempArena,1);

    u32 TotalVertexPoints = TotalXTiles * TotalZTiles;
    v3 * VertexPoints = PushArray(TempArena,TotalVertexPoints,v3);

    r32 CellDim = World->GridCellDimInMeters.x;
    r32 CellHalfDim = CellDim * 0.5f;

    // Mapping cells coordinates to range [1,n]
    // the gradient between buckets is based on current worldP offset
    r32 Density = 10.0f;
    r32 DensityMultiply = (1.0f / Density);

    // Normalize WOrldP offset to [0,1]
    r32 UnilateralWorldPOffsetX = (WorldP._Offset.x + CellHalfDim) * World->OneOverGridCellDimInMeters.x;
    r32 UnilateralWorldPOffsetZ = (WorldP._Offset.z + CellHalfDim) * World->OneOverGridCellDimInMeters.x;
    //Logn("Offx: %f Offz: %f",WorldPOffsetX, WorldPOffsetZ);
    
    r32 CellDimOffsetX = DensityMultiply * UnilateralWorldPOffsetX;
    r32 CellDimOffsetZ = DensityMultiply * UnilateralWorldPOffsetZ;
    v3 GroundP = V3((r32)WorldP.x * DensityMultiply,(r32)WorldP.y * DensityMultiply,(r32)WorldP.z * DensityMultiply);
    //v3 GroundP = V3((r32)WorldP.x ,(r32)WorldP.y ,(r32)WorldP.z );

    u32 VertexIndex = 0;
    r32 MaxX = 0;
    r32 MaxZ = 0;
    r32 MinX = 0;
    r32 MinZ = 0;

    r32 MinY = 0;
    r32 MaxY = 0;

    r32 MaxPx = (r32)(fabs((r32)WorldP.x) + XTiles - 1) * DensityMultiply + (1.0f - UnilateralWorldPOffsetX) * DensityMultiply;
    r32 MaxPz = (r32)(fabs((r32)WorldP.z) + ZTiles - 1) * DensityMultiply + (1.0f - UnilateralWorldPOffsetZ) * DensityMultiply;
    // TODO: how?
    r32 MaxPy = maxval(MaxPx, MaxPz) + (r32)fabs((r32)WorldP.y - 1.0f) * DensityMultiply;
    //v3 OneOverMaxRangeValues = V3(1.0f / (MaxPx * 2.0f), 1.0f / (MaxPy * 2.0f), 1.0f / (MaxPz * 2.0f));
    v3 OneOverMaxRangeValues = V3(1.0f / MaxPx, 1.0f / MaxPy, 1.0f / MaxPz);

    for (i32 TileX = -XTiles;
                TileX < XTiles;
                ++TileX)
    {
        r32 Px = (r32)(WorldP.x + TileX) * DensityMultiply + CellDimOffsetX;
        MaxX = maxval(Px - GroundP.x,MaxX);
        MinX = minval(Px - GroundP.x,MinX);
        for (i32 TileZ = -ZTiles;
                TileZ < ZTiles;
                ++TileZ)
        {
            r32 Pz = (r32)(WorldP.z + TileZ) * DensityMultiply + CellDimOffsetZ;
            MaxZ = maxval(Pz - GroundP.z,MaxZ);
            MinZ = minval(Pz - GroundP.z,MinX);
            r32 Py = perlin(Px, Pz);
            MaxY = maxval(Py - GroundP.y,MaxY);
            MinY = minval(Py - GroundP.y,MinY);
            v3 Vertex = V3(Px,Py,Pz) - GroundP;
            //Vertex.y = Clamp(Vertex.y,0,1);
            // TODO: I dnt need to normalize else will move center mesh and scale incorrectly
            // normalize [0,1]
            //Vertex = VectorMultiply((Vertex + V3(MaxPx, MaxPy, MaxPz)),OneOverMaxRangeValues);
            Vertex = VectorMultiply(Vertex,OneOverMaxRangeValues);
            //Vertex.x = Clamp(Vertex.x,0,1.0f);
            //Vertex.z = Clamp(Vertex.z,0,1.0f);
            //Assert((Vertex.x >= 0) & (Vertex.y >= 0) & (Vertex.z >= 0));
            *(VertexPoints + VertexIndex) = Vertex;
            Logn("Vertex:" STRP,FP(Vertex));
            ++VertexIndex;
        }
    }
    //Logn("Max x: %10f Max z: %10f Max y: %10f",MaxX, MaxZ, MaxY);
    //Logn("Min x: %10f Min z: %10f Min y: %10f",MinX, MinZ, MinY);

    v3 VUp = V3(0,1.0f,0);
    v4 Yellow = V4(1.0f,1.0f,0,1.0f);
    vertex_point * Vertices = (vertex_point *)PushSize(TempArena, MeshSize);
    u32 CountVertices = 0;
    u32 Stride = TotalZTiles;

    for (i32 TileX = 0;
                TileX < (TotalXTiles - 1);
                ++TileX)
    {
        // last Z tile is done in the prior row
        for (i32 TileZ = 0;
                TileZ < TotalZTiles;
                ++TileZ)
        {
            // right and up
            u32 OffsetX = 1;
            u32 OffsetZ = Stride;

            if (TileZ == (TotalZTiles - 1))
            {
                OffsetX = Stride;
                OffsetZ = Stride - 1;
            }

            v3 * va = VertexPoints + (Stride * TileX) + TileZ;
            v3 * vb = va + OffsetX;
            v3 * vc = va + OffsetZ;

            Vertices[CountVertices].P = *va;
            Vertices[CountVertices].N = VUp;
            Vertices[CountVertices].Color = Yellow;
            ++CountVertices;

            Vertices[CountVertices].P = *vb;
            Vertices[CountVertices].N = VUp;
            Vertices[CountVertices].Color = Yellow;
            ++CountVertices;

            Vertices[CountVertices].P = *vc;
            Vertices[CountVertices].N = VUp;
            Vertices[CountVertices].Color = Yellow;
            ++CountVertices;

            if (InBetweenExcl(TileZ,0,Stride - 1))
            {
                OffsetX = Stride;
                OffsetZ = Stride - 1;

                vb = va + OffsetX;
                vc = va + OffsetZ;

                Vertices[CountVertices].P = *va;
                Vertices[CountVertices].N = VUp;
                Vertices[CountVertices].Color = Yellow;
                ++CountVertices;

                Vertices[CountVertices].P = *vb;
                Vertices[CountVertices].N = VUp;
                Vertices[CountVertices].Color = Yellow;
                ++CountVertices;

                Vertices[CountVertices].P = *vc;
                Vertices[CountVertices].N = VUp;
                Vertices[CountVertices].Color = Yellow;
                ++CountVertices;
            }
        }
    }


    RenderPushVertexData(Vertices,MeshSize, VertexBufferBeginOffset); 

    EndTempArena(TempArena,1);

    MeshGroup->Loaded        = true;  // b32 Loaded;
    MeshGroup->LoadInProcess = false; // b32 LoadInProcess;
}
