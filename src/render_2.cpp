#include "math.h"
#include "game.h"
#include "vulkan_initializer.h"
#include "model_loader.cpp"
#include "data_load.h"
#include "collision.h"



inline void Translate(m4 &M,v3 P);
void RotateFill(m4 * M, real32 AngleX, real32 AngleY, real32 AngleZ);

inline entity_transform *
GetEntityTransform(game_state * GameState,entity Entity)
{
    entity_transform * T = &GameState->EntitiesTransform[Entity.ID];
    return T;
}

inline v3
GetMatrixRight(m4 &RotationMatrix)
{
    v3 D = V3(RotationMatrix[0].x,RotationMatrix[1].x,RotationMatrix[2].x);
    
    return D;
}

inline v3
GetEntityPos(game_state * GameState,entity Entity)
{
    v3 P = GameState->EntitiesTransform[Entity.ID].WorldP[3].xyz;
    
    return P;
}

inline v3
GetMatrixPos(m4 &M)
{
    // Render negates view position.
    v3 P = M[3].xyz;
    
    return P;
}

inline v3
GetViewPos(game_state * GameState)
{
    // Render negates view position.
    v3 P = -GameState->ViewMoveMatrix[3].xyz;
    
    return P;
}

inline v3
GetMatrixDirection(m4 &RotationMatrix)
{
    v3 D = V3(RotationMatrix[0].z,RotationMatrix[1].z,RotationMatrix[2].z);
    
    return D;
}

m4
ProjectionMatrix(real32 FOV,real32 AspectRatio, real32 n, real32 f)
{
    m4 m = {};

    real32 HalfTanFOV = (real32)(tan(FOV * 0.5f));
#if 1
    real32 A = -(f + n) / (f - n);
    real32 B = (-2*f*n) / (f - n);
#else
    real32 A = -f / (f - n);
    real32 B = (-2*f*n) / (f - n);
#endif

    m[0].x = 1.0f / (HalfTanFOV * AspectRatio); // (2 * n) / (r - l);
    // m.c20 = 0; //(r + l) / (r - l);
    m[1].y = 1.0f / (HalfTanFOV); // (2 * n) / (t - b);
    // m.c21 = 0; //(t + b) / (t - b);
    m[2].z = A;
    m[3].z = B;
    m[2].w = -1.0f; // -Pz
    m[3].w = 0;

    return m;
} 

void
WorldInitializeView(game_state * GameState,
               real32 FOV,
               int32 ScreenWidth, int32 ScreenHeight, 
               real32 n, real32 f, 
               v3 P, v3 WorldUp = DEFAULT_WORLD_UP)
{
    GameState->WorldUp = WorldUp;
    GameState->Projection = ProjectionMatrix(FOV,((real32)ScreenWidth / (real32)ScreenHeight), n,f);

    // Depends on world up
    Translate(GameState->ViewMoveMatrix,-P);

    RotateFill(&GameState->ViewRotationMatrix, 0, 0, 0);
}

void
BeginRender(game_state * GameState, v4 ClearColor)
{
    WaitForRender();

    RenderBeginPass(ClearColor);

    RenderSetPipeline(GameState->PipelineIndex);
}

void
EndRender(game_state * GameState)
{
    RenderEndPass();
}


memory_arena *
ThreadBeginArena(thread_memory_arena * ThreadArena)
{
    Assert(ThreadArena->InUse);
    memory_arena * Arena = &ThreadArena->Arena;
    Assert(Arena->Base && Arena->MaxSize > 0);
    Arena->CurrentSize = 0;
    return Arena;
}
void
ThreadEndArena(thread_memory_arena * ThreadArena)
{
    Assert(ThreadArena->InUse);
    ThreadArena->Arena.CurrentSize = 0;
    ThreadArena->InUse = false;
}

struct async_load_mesh
{
    game_memory * Memory;
    game_state * GameState;
    char * Path;
    mesh * Mesh;
    thread_memory_arena * ThreadArena;
    void * BufferVertices;
    uint32 BaseOffset;
};

void
PushMeshSize(memory_arena * Arena, uint32 DataSize, uint32 InstanceCount)
{
    uint32 TotalSize = (DataSize * InstanceCount);
    uint32 Align = RenderGetVertexMemAlign() - 1;
    TotalSize = (TotalSize + Align) &  ~Align;

    Assert((Arena->CurrentSize + TotalSize) < Arena->MaxSize);

    Arena->CurrentSize += TotalSize;
}

THREAD_WORK_HANDLER(LoadMesh)
{
    async_load_mesh * WorkData = (async_load_mesh *)Data;
    Assert(WorkData->ThreadArena);

    game_state * GameState = WorkData->GameState;
    mesh * Mesh = WorkData->Mesh;
    Assert(Mesh);

    memory_arena * Arena = &WorkData->ThreadArena->Arena;
    Assert(!Mesh->Loaded);

    int32 Result = -1;
    file_contents GetFileResult = GetFileContents(WorkData->Memory, Arena,WorkData->Path);

    if (GetFileResult.Success)
    {
        obj_file_header Header = 
            ReadObjFileHeader((const char *)GetFileResult.Base, GetFileResult.Size);
        mesh TempMesh   = 
            CreateMeshFromObjHeader(Arena,
                    WorkData->BufferVertices,
                    Header, 
                    (const char *)GetFileResult.Base, GetFileResult.Size);

        Mesh->Vertices       = TempMesh.Vertices; // vertex_point * Vertices;
        Mesh->VertexSize     = TempMesh.VertexSize; // uint32   VertexSize;
        Mesh->Indices        = TempMesh.Indices; // Typedef * Indices;
        Mesh->IndicesSize    = TempMesh.IndicesSize; // uint32   IndicesSize;

        // TODO GPU barrier sync
        RenderPushVertexData(Mesh->Vertices,Mesh->VertexSize, WorkData->BaseOffset);
        //RenderPushVertexData(Mesh->Vertices,Mesh->VertexSize, 1);
        //RenderPushIndexData(&GameState->IndicesArena, Mesh->Indices,Mesh->IndicesSize, 1);

        COMPILER_DONOT_REORDER_BARRIER;
        Mesh->Loaded = true;
    }

#if 0
    v3 dir[3] = {
        {1,0,0},{0,1,0},{0,0,1}
    };

    for (uint32 VectorDirIndex = 0;
            VectorDirIndex < 3;
            ++VectorDirIndex)
    {
        int imin, imax;
        ExtremePointsAlongDirection(dir[VectorDirIndex], Mesh->Vertices, Mesh->VertexSize / sizeof(vertex_point), &imin, &imax);
        v3 MinP = Mesh->Vertices[imin].P;
        v3 MaxP = Mesh->Vertices[imax].P;
    }
#endif

    ThreadEndArena(WorkData->ThreadArena);
}

/*
 *  This is single thread proc
 *  Fetching thread arenas should be done in the main thread
 *  and no available arena case must be handle properly
 *  Any critical step that must be guaranteed to be handle
 *  in the current frame must be done using CompleteQueue 
 */
thread_memory_arena *
GetThreadArena(game_state * GameState)
{
    thread_memory_arena * ThreadArena = 0;

    for (uint32 ThreadArenaIndex = 0;
                ThreadArenaIndex < GameState->LimitThreadArenas;
                ++ThreadArenaIndex)
    {
        thread_memory_arena * TestThreadArena = GameState->ThreadArena + ThreadArenaIndex;
        if (!TestThreadArena->InUse)
        {
            TestThreadArena->InUse = true;
            ThreadArena = TestThreadArena;
            break;
        }
    }

    return ThreadArena;
}

inline uint32
StrLen(const char * c)
{
    uint32 ci = 0;
    for (;
            (c[ci] != 0);
            ++ci)
    { }
    return ci;
}

inline void
CopyStr(char * DestStr,const char * SrcStr ,uint32 Length)
{
    for (uint32 ci = 0;
                ci < Length;
                ++ci)
    {
        DestStr[ci] = SrcStr[ci];
    }
}

mesh *
GetMesh(game_memory * Memory, game_state * GameState,uint32 ID)
{

    mesh * Mesh = (GameState->Meshes + ID);

    if (!Mesh->Loaded && !Mesh->LoadInProcess)
    {
        const char * Paths[2] = {
            "assets\\cube.obj",
            "assets\\human_male_triangles.obj"
        };
        const uint32 MeshSizes[2] = {
            36 * sizeof(vertex_point),
            4200 * sizeof(vertex_point),
        };
        thread_memory_arena * ThreadArena = GetThreadArena(GameState);
        if (ThreadArena)
        {
            Mesh->LoadInProcess = true;

            memory_arena * Arena = ThreadBeginArena(ThreadArena);

            async_load_mesh * Data = PushStruct(Arena,async_load_mesh);
            Data->Memory    = Memory;                  // game_memory * Memory;
            Data->GameState = GameState;               // game_state * GameState;
            uint32 LenPath  = StrLen(Paths[ID]) + 1;
            Data->Path      = (char *)PushSize(Arena,LenPath); // Char_S * Path;
            CopyStr(Data->Path, Paths[ID], LenPath);
            Data->ThreadArena = ThreadArena;
            Data->Mesh = Mesh;

            uint32 MeshSize = MeshSizes[ID];
            // For multi thread
            // Get current base for arena
            Data->BaseOffset = GameState->MeshesArena.CurrentSize;
            Data->BufferVertices = GameState->MeshesArena.Base + GameState->MeshesArena.CurrentSize;
            // Increment MeshArena with the known total size required
            PushMeshSize(&GameState->MeshesArena, MeshSize, 1);

            // TODO I don't like this
            // Multi thread mesh load, push arena size based on size
            // of the current requested mesh
            Mesh->OffsetVertices = Data->BaseOffset;
            PushSize(&GameState->VertexArena, MeshSize);
            

            //Mesh->OffsetIndices = GameState->IndicesArena.CurrentSize;
            
            Memory->AddWorkToWorkQueue(Memory->RenderWorkQueue , LoadMesh,Data);
        }
    }

    return Mesh;
}

void
UpdateView(game_state * GameState)
{
    GameState->ViewTransform = GameState->ViewRotationMatrix * GameState->ViewMoveMatrix;
}

void
RenderEntities(game_memory * Memory, game_state * GameState)
{
    v3 SourceLight = V3(0,10.0f,0);
    v4 Color = V4(1.0f,0.5f,0.2f,1.0f);

    UpdateView(GameState);

    v3 ViewPos = GetViewPos(GameState);
    v3 ViewDirection = GetMatrixDirection(GameState->ViewRotationMatrix);

    for (uint32 EntityIndex = 0;
                EntityIndex < GameState->TotalEntities;
                ++EntityIndex)
    {
        uint32 EntityID = GameState->Entities[EntityIndex].ID;
        entity_transform * T = (GameState->EntitiesTransform + EntityID);

        v3 ViewToEntity = GetMatrixPos(T->WorldP) - ViewPos;
        real32 ViewProjectionOntoEntity = Inner(ViewToEntity,ViewDirection); 

        // We are culling entities behind camera
        // TODO: Fix ground as it will be behind camera and dissapear
        //if (ViewProjectionOntoEntity >= 0.0f)
        if (1.0f)
        {
            render_3D * R3D = GameState->Render3D + EntityID;
            mesh * Mesh = GetMesh(Memory,GameState,R3D->MeshID);
            //if (Mesh->Loaded && EntityID == 1)
            if (Mesh->Loaded)
            {
                m4 ModelTransform = T->WorldT;

                mesh_push_constant Constants;
                m4 MVP = GameState->Projection * GameState->ViewTransform * ModelTransform;

                Constants.RenderMatrix = MVP;
                Constants.SourceLight = SourceLight;
                Constants.Model = ModelTransform;
                v4 ColorDebug = V4(V3(0.0f),1.0f);
                ColorDebug._V[EntityID % 3] = 1.0f;
                Constants.DebugColor = ColorDebug;
                Constants.DebugColor = V4(R3D->Color,1.0f);

                RenderPushVertexConstant(sizeof(mesh_push_constant),(void *)&Constants);
                //RenderPushMesh(1,(Mesh->IndicesSize / sizeof(uint16)),Mesh->OffsetVertices,Mesh->OffsetIndices);
                //RenderPushMesh(1, Mesh->VertexSize / sizeof(vertex_point), 0);
                RenderPushMesh(1, Mesh->VertexSize / sizeof(vertex_point), Mesh->OffsetVertices);
            }
        }
    }
}

inline void
Translate(m4 &M,v3 P)
{
    M.Columns[0] = {1, 0 , 0 , 0};
    M.Columns[1] = {0, 1 , 0 , 0};
    M.Columns[2] = {0, 0 , 1 , 0};
    M.Columns[3] = {P.x, P.y , P.z , 1};
}

void
RotateEntity(game_state * GameState,entity Entity,real32 Pitch, real32 Yaw)
{
    real32 cx = cosf(Pitch);
    real32 sx = sinf(Pitch);
    real32 cy = cosf(Yaw);
    real32 sy = sinf(Yaw);

    v3 xaxis = { cy, 0, -sy};
    v3 yaxis = { sy* sx, cx, cy * sx};
    v3 zaxis = { sy* cx, -sx, cx * cy};

    m4 R =  {
        V4( xaxis.x, yaxis.x, zaxis.x, 0 ),
        V4( xaxis.y, yaxis.y, zaxis.y, 0 ),
        V4( xaxis.z, yaxis.z, zaxis.z, 0 ),
        V4( 0,0,0,1 )
    };

    entity_transform * T = GetEntityTransform(GameState,Entity);

    T->LocalR = R * T->LocalR;
}

inline real32
GetYawFromRotationMatrix(m4 * R)
{
    real32 Yaw = atanf(R->Columns[0].y / R->Columns[0].x);
    return Yaw;
}

inline void
UpdateEntityYaw(game_state * GameState, entity * Entity)
{
    m4 * R = &GameState->EntitiesTransform[Entity->ID].LocalR;
    real32 Yaw = GetYawFromRotationMatrix(R); 
    GameState->EntitiesTransform[Entity->ID].Yaw = Yaw;
}

void
EntityLookAt(game_state * GameState,entity Entity, v3 P)
{
    v3 D = Normalize(P - GetEntityPos(GameState,Entity));
    v3 WorldUp = GameState->WorldUp;
    v3 Right = Normalize(Cross(D,WorldUp));
    v3 Up = Cross(Right,D);

    m4 R = {};
    R[0].x = Right.x;
    R[0].y = Up.x;
    R[0].z = D.x;
    R[0].w = 0;

    R[1].x = Right.y;
    R[1].y = Up.y;
    R[1].z = D.y;
    R[1].w = 0;

    R[2].x = Right.z;
    R[2].y = Up.z;
    R[2].z = D.z;
    R[2].w = 0;

    R[3].x = 0;
    R[3].y = 0;
    R[3].z = 0;
    R[3].w = 1;

    GameState->EntitiesTransform[Entity.ID].LocalR = R;
    UpdateEntityYaw(GameState,&Entity);
}

int32
ViewLookAt(game_state * GameState, v3 P, v3 TargetP)
{
    v3 Right, Up, Out;

    Out = TargetP - P;

    real32 LengthOut = Length(Out);

    if (LengthOut < 0.000001)
    {
        return -1;
    }

    Out = Out / LengthOut;

    Up = GameState->WorldUp - (Inner(GameState->WorldUp, Out)*Out);

    real32 LengthSqrUp = LengthSqr(Up);

    // too close
    Assert(LengthSqrUp > 0.000001f);

    Up = Up / sqrtf(LengthSqrUp);

    Right = Cross(Out,Up);
    //Right = Cross(Up,Out);

    GameState->ViewRotationMatrix[0].x = Right.x;
    GameState->ViewRotationMatrix[0].y = Up.x;
    GameState->ViewRotationMatrix[0].z = -Out.x;
    GameState->ViewRotationMatrix[0].w = 0;

    GameState->ViewRotationMatrix[1].x = Right.y;
    GameState->ViewRotationMatrix[1].y = Up.y;
    GameState->ViewRotationMatrix[1].z = -Out.y;
    GameState->ViewRotationMatrix[1].w = 0;

    GameState->ViewRotationMatrix[2].x = Right.z;
    GameState->ViewRotationMatrix[2].y = Up.z;
    GameState->ViewRotationMatrix[2].z = -Out.z;
    GameState->ViewRotationMatrix[2].w = 0;

    GameState->ViewRotationMatrix[3].x = 0;
    GameState->ViewRotationMatrix[3].y = 0;
    GameState->ViewRotationMatrix[3].z = 0;
    GameState->ViewRotationMatrix[3].w = 1;

    // opposite to the scene entities
    Translate(GameState->ViewMoveMatrix, -P);

    return 0;
}


void
RotateFill(m4 * M, real32 AngleX, real32 AngleY, real32 AngleZ)
{ 
    real32 cx = cosf(AngleX);
    real32 sx = sinf(AngleX);
    real32 cy = cosf(AngleY);
    real32 sy = sinf(AngleY);
    real32 cz = cosf(AngleZ);
    real32 sz = sinf(AngleZ);

    m4 AxisZ = {
        1, 0 , 0, 0,
        0, cx, -sx, 0,
        0, sx, cx, 0,
        0, 0, 0, 1
    };
    m4 AxisX = {
        cy, 0, sy, 0,
        0,  1,  0,  0,
        -sy, 0,  cy, 0,
        0,  0,  0,  1
    };
    m4 AxisY = {
        cz, -sz, 0, 0,
        sz,  cz, 0, 0,
        0,    0,  1, 0,
        0,    0,  0, 1
    };

    //*M = AxisZ * AxisY * AxisX;
    *M = AxisX * AxisY * AxisZ;
}

void
MoveViewRight(game_state * GameState,real32 N)
{
    v3 P = GetViewPos(GameState);
    v3 Right = GetMatrixRight(GameState->ViewRotationMatrix);
    // do not alter Y
    //Right.y = 0.0f;
    v3 R = P + (N * Right);
    Translate(GameState->ViewMoveMatrix, -R);
    UpdateView(GameState);
}
void
MoveViewForward(game_state * GameState,real32 N)
{
    v3 P = GetViewPos(GameState);
    v3 Out = GetMatrixDirection(GameState->ViewRotationMatrix);
    // do not alter Y
    //Out.y = 0.0f;
    v3 R = P + (N * Out);
    Translate(GameState->ViewMoveMatrix, -R);
    UpdateView(GameState);
}

void
RotateRight(m4 * M,real32 Angle)
{
    real32 c = cosf(-Angle);
    real32 s = sinf(-Angle);
    m4 R = {
        c, 0 , s , 0,
        0, 1, 0 , 0,
        -s, 0 , c, 0,
        0, 0, 0, 1
    };

    *M = R * (*M);
}

void
RotateUp(m4 * M,real32 Angle)
{
    real32 c = cosf(-Angle);
    real32 s = sinf(-Angle);
    m4 R = {
        1, 0 , 0, 0,
        0, c, -s, 0,
        0, s, c, 0,
        0, 0, 0, 1
    };

    *M = R * (*M);
}

void
SetViewBehindObject(game_state * GameState, v3 T, v3 D, real32 Separation, real32 HeightOverObject = 0.0f)
{
    D = (D / Length(D));

    v3 V = T - D * Separation + V3(0,HeightOverObject,0);

    ViewLookAt(GameState,V,T);
}
