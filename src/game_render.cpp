#include "game.h"

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

void
UpdateView(game_state * GameState)
{
    GameState->ViewTransform = GameState->ViewRotationMatrix * GameState->ViewMoveMatrix;
}


void
RenderEntities(game_memory * Memory, game_state * GameState)
{

    START_CYCLE_COUNT(render_entities);

    //v3 SourceLight = V3(0,10.0f,0);
    v3 SourceLight = GameState->DebugSourceLightP;
    v4 Color = V4(1.0f,0.5f,0.2f,1.0f);

    UpdateView(GameState);

    v3 ViewPos = GetViewPos(GameState);
    v3 ViewDirection = GetMatrixDirection(GameState->ViewRotationMatrix);
    simulation * Sim = GameState->Simulation;

    simulation_iterator SimIter = BeginSimIterator(&GameState->World, Sim);

    for (entity * Entity = SimIter.Entity;
            Entity;
            Entity = AdvanceSimIterator(&SimIter))
    {
        // TODO: I don't want to render ground like this
        //       but neither I know how can I query entities/ground collision
        //       within same query otherwise
        mesh_group * MeshGroup = 0;

        MeshGroup = GetMesh(Memory,GameState,Entity->MeshID);

        if (
                IS_NOT_NULL(MeshGroup) && 
                MeshGroup->Loaded
            )
        {
            simulation_mesh_obj_transform_iterator Iterator =
                BeginSimMeshObjTransformIterator(Sim, Entity);

            for (entity_transform * MeshObjT = Iterator.T;
                    IS_NOT_NULL(MeshObjT);
                    MeshObjT = AdvanceSimMeshObjTransformIterator(&Iterator))
            {
                mesh * Mesh = MeshGroup->Meshes + Iterator.Index;

                m4 ModelTransform = MeshObjT->WorldT;

                mesh_push_constant Constants;
                m4 MVP = GameState->Projection * GameState->ViewTransform * ModelTransform;

                Constants.RenderMatrix = MVP;
                Constants.SourceLight = SourceLight;
                Constants.Model = ModelTransform;
                //v4 ColorDebug = V4(V3(0.0f),1.0f);
                //ColorDebug._V[Entity->ID.ID % 3] = 1.0f;
                //Constants.DebugColor = ColorDebug;
                Constants.DebugColor = V4(Entity->Color,1.0f);

                RenderPushVertexConstant(sizeof(mesh_push_constant),(void *)&Constants);
                //RenderPushMesh(1,(Mesh->IndicesSize / sizeof(uint16)),Mesh->OffsetVertices,Mesh->OffsetIndices);
                //RenderPushMesh(1, Mesh->VertexSize / sizeof(vertex_point), 0);
                RenderPushMesh(1, Mesh->VertexSize / sizeof(vertex_point), Mesh->OffsetVertices);
            }
            //Logn("Rendered mesh (%i) with %i objects",Entity->ID.ID, Iterator.Index);
        }
    }

    SimIter = BeginSimGroundIterator(&GameState->World, Sim);

    for (entity * Entity = SimIter.Entity;
            Entity;
            Entity = AdvanceSimGroundIterator(&SimIter))
    {
        mesh_group * MeshGroup = GameState->GroundMeshGroup + Entity->MeshID.ID;
        mesh * Mesh = MeshGroup->Meshes;

        m4 ModelTransform = Entity->Transform.WorldT;

        mesh_push_constant Constants;
        m4 MVP = GameState->Projection * GameState->ViewTransform * ModelTransform;

        Constants.RenderMatrix = MVP;
        Constants.SourceLight = SourceLight;
        Constants.Model = ModelTransform;

        Constants.DebugColor = V4(Entity->Color,1.0f);

        RenderPushVertexConstant(sizeof(mesh_push_constant),(void *)&Constants);
        RenderPushMesh(1, Mesh->VertexSize / sizeof(vertex_point), Mesh->OffsetVertices);
    }

    END_CYCLE_COUNT(render_entities);
}

#if 0
void
RenderEntities(game_memory * Memory, game_state * GameState)
{
    v3 SourceLight = V3(0,10.0f,0);
    v4 Color = V4(1.0f,0.5f,0.2f,1.0f);

    UpdateView(GameState);

    v3 ViewPos = GetViewPos(GameState);
    v3 ViewDirection = GetMatrixDirection(GameState->ViewRotationMatrix);

    for (u32 EntityIndex = 0;
                EntityIndex < GameState->TotalEntities;
                ++EntityIndex)
    {
        u32 EntityID = GameState->Entities[EntityIndex].ID;
        entity_transform * T = (GameState->EntitiesTransform + EntityID);

        v3 ViewToEntity = GetMatrixPos(T->WorldP) - ViewPos;
        r32 ViewProjectionOntoEntity = Inner(ViewToEntity,ViewDirection); 

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
#endif

void
WorldInitializeView(game_state * GameState,
               r32 FOV,
               i32 ScreenWidth, i32 ScreenHeight, 
               r32 n, r32 f, 
               v3 P, v3 WorldUp)
{
    GameState->WorldUp = WorldUp;
    GameState->Projection = ProjectionMatrix(FOV,((r32)ScreenWidth / (r32)ScreenHeight), n,f);

    // Depends on world up
    Translate(GameState->ViewMoveMatrix,-P);

    RotateFill(&GameState->ViewRotationMatrix, 0, 0, 0);
}

void
MoveViewRight(game_state * GameState,r32 N)
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
MoveViewForward(game_state * GameState,r32 N)
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
RotateFill(m4 * M, r32 AngleX, r32 AngleY, r32 AngleZ)
{ 
    r32 cx = cosf(AngleX);
    r32 sx = sinf(AngleX);
    r32 cy = cosf(AngleY);
    r32 sy = sinf(AngleY);
    r32 cz = cosf(AngleZ);
    r32 sz = sinf(AngleZ);

    m4 AxisX = {
        1, 0 , 0, 0,
        0, cx, -sx, 0,
        0, sx, cx, 0,
        0, 0, 0, 1
    };
    m4 AxisY = {
        cy, 0, sy, 0,
        0,  1,  0,  0,
        -sy, 0,  cy, 0,
        0,  0,  0,  1
    };
    m4 AxisZ = {
        cz, -sz, 0, 0,
        sz,  cz, 0, 0,
        0,    0,  1, 0,
        0,    0,  0, 1
    };

    //*M = AxisZ * AxisY * AxisX;
    *M = AxisX * AxisY * AxisZ;
}

inline v3
GetViewPos(game_state * GameState)
{
    // Render negates view position.
    v3 P = -GameState->ViewMoveMatrix[3].xyz;
    
    return P;
}

inline v3
GetMatrixRight(m4 &RotationMatrix)
{
    v3 D = V3(RotationMatrix[0].x,RotationMatrix[1].x,RotationMatrix[2].x);
    
    return D;
}


inline v3
GetMatrixPos(m4 &M)
{
    // Render negates view position.
    v3 P = M[3].xyz;
    
    return P;
}

inline v3
GetMatrixDirection(m4 &RotationMatrix)
{
    v3 D = V3(RotationMatrix[0].z,RotationMatrix[1].z,RotationMatrix[2].z);
    
    return D;
}

i32
ViewLookAt(game_state * GameState, v3 P, v3 TargetP)
{
    v3 Right, Up, Out;

    Out = TargetP - P;

    r32 LengthOut = Length(Out);

    if (LengthOut < 0.000001)
    {
        return -1;
    }

    Out = Out / LengthOut;

    Up = GameState->WorldUp - (Inner(GameState->WorldUp, Out)*Out);

    r32 LengthSqrUp = LengthSqr(Up);

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

inline r32
GetYawFromRotationMatrix(m4 * R)
{
    r32 Yaw = atanf(R->Columns[0].y / R->Columns[0].x);
    return Yaw;
}
#if 0
inline void
UpdateEntityYaw(game_state * GameState, entity * Entity)
{
    m4 * R = &GameState->EntitiesTransform[Entity->ID].LocalR;
    r32 Yaw = GetYawFromRotationMatrix(R); 
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
#endif

m4
ProjectionMatrix(r32 FOV,r32 AspectRatio, r32 n, r32 f)
{
    m4 m = {};

    r32 HalfTanFOV = (r32)(tan(FOV * 0.5f));
#if 1
    r32 A = -(f + n) / (f - n);
    r32 B = (-2*f*n) / (f - n);
#else
    r32 A = -f / (f - n);
    r32 B = (-2*f*n) / (f - n);
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

i32
LoadShader(game_memory * Memory,memory_arena * Arena,const char * Filepath)
{
    i32 Result = -1;
    file_contents GetFileResult = GetFileContents(Memory, Arena,Filepath);
    if (GetFileResult.Success)
    {
        Result = RenderCreateShaderModule((char *)GetFileResult.Base, (size_t)GetFileResult.Size);
        // data lives on gpu side now
        Arena->CurrentSize -= GetFileResult.Size;
    }
    return Result;
}

void
CreatePipeline(game_memory * Memory,game_state * GameState)
{
    GameState->ShadersArena.CurrentSize = 0;
    RenderFreeShaders();
    VulkanDestroyPipeline();

    GameState->VertexShaders[shader_type_vertex_default] 
        = LoadShader(Memory,&GameState->ShadersArena,"shaders\\triangle.vert");
    GameState->FragmentShaders[shader_type_fragment_default] 
        = LoadShader(Memory,&GameState->ShadersArena,"shaders\\triangle.frag");

    GameState->PipelineIndex = 
        RenderCreatePipeline(GameState->VertexShaders[shader_type_vertex_default], 
                GameState->FragmentShaders[shader_type_fragment_default]);

    // TODO: destroy shaders after pipeline creation

    if (GameState->PipelineIndex < 0)
    {
        Log("Error during creation of main pipeline\n");
    }

    Assert(GameState->PipelineIndex >= 0);
}
