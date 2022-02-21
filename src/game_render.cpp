#include "game.h"
#include "data_load.h"



#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

inline v3
GetViewPos(render_controller * Renderer)
{
    // Render negates view position.
    v3 P = -Renderer->ViewMoveMatrix[3].xyz;
    
    return P;
}

render_controller
NewRenderController(memory_arena * Arena,
                    v3 WorldUp,
                    r32 FOV,
                    i32 ScreenWidth, i32 ScreenHeight, 
                    r32 n, r32 f,
                    v3 StartP)
{
    render_controller Renderer;

    Renderer.WorldUp            = WorldUp; // RECORD   WorldUp;

    Translate(Renderer.ViewMoveMatrix, -StartP);

    RotateFill(&Renderer.ViewRotationMatrix, 0, 0, 0);
    Renderer.ViewRotationMatrix = M4();
    Renderer.ViewTransform      = M4();
    Renderer.Projection         = ProjectionMatrix(FOV,((r32)ScreenWidth / (r32)ScreenHeight), n,f);
    Renderer.Arena              = Arena;

    Renderer.VertexArena = RenderGetMemoryArena();
    Renderer.IndicesArena = RenderGetMemoryArena();

    for (i32 i = 0; i < ArrayCount(Renderer.Pipelines); ++i)
    {
        Renderer.Pipelines[i] = -1;
    }

    for (u32 RenderPassIndex = 0;
                RenderPassIndex < ArrayCount(Renderer.RenderPasses);
                ++RenderPassIndex)
    {
        render_pass * RenderPass = Renderer.RenderPasses + RenderPassIndex;
        RenderPass->Limit = 4096;
        RenderPass->Count = 0;
        RenderPass->Units = PushArray(Renderer.Arena,RenderPass->Limit,render_unit);
    }
#if DEBUG
    render_pass * RenderPass = &Renderer.RenderPassDebug;
    RenderPass->Limit = 4096;
    RenderPass->Count = 0;
    RenderPass->Units = PushArray(Renderer.Arena,RenderPass->Limit,render_unit);
#endif

    return Renderer;
}

void
UpdateView(render_controller * Renderer)
{
    Renderer->ViewTransform = Renderer->ViewRotationMatrix * Renderer->ViewMoveMatrix;
}

void
BeginRender(render_controller * Renderer, v4 ClearColor)
{
    WaitForRender();

    RenderBeginPass(ClearColor);

    UpdateView(Renderer);
}

void
EndRender(render_controller * Renderer)
{
    RenderEndPass();

    for (u32 RenderPassIndex = 0;
                RenderPassIndex < ArrayCount(Renderer->RenderPasses);
                ++RenderPassIndex)
    {
        render_pass * RenderPass = Renderer->RenderPasses + RenderPassIndex;
        RenderPass->Count = 0;
    }
#if DEBUG
    Renderer->RenderPassDebug.Count = 0;
#endif
}

void
RenderDraw(game_state * GameState, game_memory * Memory,render_controller * Renderer)
{
    for (u32 RenderPassIndex = 0;
                RenderPassIndex < ArrayCount(Renderer->RenderPasses);
                ++RenderPassIndex)
    {
        render_pass * RenderPass = Renderer->RenderPasses + RenderPassIndex;

        RenderSetPipeline(Renderer->Pipelines[RenderPassIndex]);

        mesh_group * LastMeshGroup = 0;

        for (u32 UnitIndex = 0;
                UnitIndex < RenderPass->Count;
                ++UnitIndex)
        {
            render_unit * Unit = RenderPass->Units + UnitIndex;

            mesh_group * MeshGroup = 0;

            MeshGroup = GetMesh(Memory,GameState,Unit->MeshID);

            mesh * Mesh = MeshGroup->Meshes + 0;

            u32 MeshSize = Mesh->VertexSize / sizeof(vertex_point);

            if (LastMeshGroup != MeshGroup)
            {
                RenderBindMesh(MeshSize, Mesh->OffsetVertices);
                LastMeshGroup = MeshGroup;
            }

            mesh_push_constant Constants;

            m4 MVP = Renderer->Projection * Renderer->ViewTransform * Unit->ModelTransform;

            Constants.RenderMatrix = MVP;
            Constants.SourceLight = V3(0,10.0f,0);
            Constants.Model = Unit->ModelTransform;
            Constants.DebugColor = Unit->Color;

            RenderPushVertexConstant(sizeof(mesh_push_constant),(void *)&Constants);
            RenderDrawMesh(MeshSize);
        }
    }
#if DEBUG


#endif
}
void
RenderDrawGround(game_state * GameState,render_controller * Renderer, simulation * Sim)
{
    simulation_iterator SimIter = BeginSimGroundIterator(&GameState->World, Sim);
    v3 SourceLight = GetViewPos(Renderer) + V3(0,1,0) ;

    for (entity * Entity = SimIter.Entity;
            Entity;
            Entity = AdvanceSimGroundIterator(&SimIter))
    {
        mesh_group * MeshGroup = GameState->GroundMeshGroup + Entity->MeshID.ID;
        mesh * Mesh = MeshGroup->Meshes;

        m4 ModelTransform = Entity->Transform.WorldT;

        mesh_push_constant Constants;
        m4 MVP = Renderer->Projection * Renderer->ViewTransform * ModelTransform;

        Constants.RenderMatrix = MVP;
        Constants.SourceLight = SourceLight;
        Constants.Model = ModelTransform;

        Constants.DebugColor = V4(Entity->Color,1.0f - Entity->Transparency);

        u32 MeshSize = Mesh->VertexSize /sizeof(vertex_point);
        RenderPushVertexConstant(sizeof(mesh_push_constant),(void *)&Constants);
        RenderBindMesh(MeshSize, Mesh->OffsetVertices);
        RenderDrawMesh(MeshSize);
    }
}

void
PushDraw(render_controller * Renderer, material_type Material, m4 * ModelT, mesh_id MeshID, v3 Color, r32 Transparency)
{

    render_pass * RenderPass = Renderer->RenderPasses + Material;

    Assert(RenderPass->Count <= RenderPass->Limit);

    render_unit * Unit = RenderPass->Units + RenderPass->Count++;
    Unit->ModelTransform = *ModelT;
    Unit->MeshID = MeshID;
    Unit->Color = V4(Color, 1.0f - Transparency);
}

#if DEBUG
void
PushDrawDebug(render_controller * Renderer,entity * Entity)
{
    render_pass * RenderPass = &Renderer->RenderPassDebug;

    Assert(RenderPass->Count <= RenderPass->Limit);

    render_unit * Unit = RenderPass->Units + RenderPass->Count++;
    Unit->ModelTransform = Entity->Transform.WorldT;
    mesh_id MeshID = {0};
    Unit->MeshID = MeshID;
    Unit->Color = V4(Entity->Color, 1.0f - Entity->Transparency);
}
#endif


void
PushDrawEntity(render_controller * Renderer,entity * Entity)
{
    PushDraw(Renderer, Entity->Material, &Entity->Transform.WorldT, Entity->MeshID, Entity->Color, Entity->Transparency);
}


void
PushDrawSimulation(game_memory * Memory, game_state * GameState, simulation * Sim)
{

    START_CYCLE_COUNT(render_entities);

    //v3 SourceLight = V3(0,10.0f,0);
    //v3 SourceLight = GameState->DebugSourceLightP;
    v4 Color = V4(1.0f,0.5f,0.2f,1.0f);

    v3 SourceLight = GetViewPos(&GameState->Renderer) + V3(0,1,0) ;

    simulation_iterator SimIter = BeginSimIterator(&GameState->World, Sim);

    for (entity * Entity = SimIter.Entity;
            Entity;
            Entity = AdvanceSimIterator(&SimIter))
    {
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
                PushDrawEntity(&GameState->Renderer,Entity);
            }
        }
    }


    END_CYCLE_COUNT(render_entities);
}


void
MoveViewRight(render_controller * Renderer,r32 N)
{
    v3 P = GetViewPos(Renderer);
    v3 Right = GetMatrixRight(Renderer->ViewRotationMatrix);
    // do not alter Y
    //Right.y = 0.0f;
    v3 R = P + (N * Right);
    Translate(Renderer->ViewMoveMatrix, -R);
    UpdateView(Renderer);
}
void
MoveViewForward(render_controller * Renderer,r32 N)
{
    v3 P = GetViewPos(Renderer);
    v3 Out = GetMatrixDirection(Renderer->ViewRotationMatrix);
    // do not alter Y
    //Out.y = 0.0f;
    v3 R = P + (N * Out);
    Translate(Renderer->ViewMoveMatrix, -R);
    UpdateView(Renderer);
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
ViewLookAt(render_controller * Renderer, v3 P, v3 TargetP)
{
    v3 Right, Up, Out;

    Out = TargetP - P;

    r32 LengthOut = Length(Out);

    if (LengthOut < 0.000001)
    {
        return -1;
    }

    Out = Out / LengthOut;

    Up = Renderer->WorldUp - (Inner(Renderer->WorldUp, Out)*Out);

    r32 LengthSqrUp = LengthSqr(Up);

    // too close
    Assert(LengthSqrUp > 0.000001f);

    Up = Up / sqrtf(LengthSqrUp);

    Right = Cross(Out,Up);
    //Right = Cross(Up,Out);

    Renderer->ViewRotationMatrix[0].x = Right.x;
    Renderer->ViewRotationMatrix[0].y = Up.x;
    Renderer->ViewRotationMatrix[0].z = -Out.x;
    Renderer->ViewRotationMatrix[0].w = 0;

    Renderer->ViewRotationMatrix[1].x = Right.y;
    Renderer->ViewRotationMatrix[1].y = Up.y;
    Renderer->ViewRotationMatrix[1].z = -Out.y;
    Renderer->ViewRotationMatrix[1].w = 0;

    Renderer->ViewRotationMatrix[2].x = Right.z;
    Renderer->ViewRotationMatrix[2].y = Up.z;
    Renderer->ViewRotationMatrix[2].z = -Out.z;
    Renderer->ViewRotationMatrix[2].w = 0;

    Renderer->ViewRotationMatrix[3].x = 0;
    Renderer->ViewRotationMatrix[3].y = 0;
    Renderer->ViewRotationMatrix[3].z = 0;
    Renderer->ViewRotationMatrix[3].w = 1;

    // opposite to the scene entities
    Translate(Renderer->ViewMoveMatrix, -P);

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
RenderPushTexture(game_state * GameState, void * Data, u32 Width, u32 Height)
{
    i32 VulkanPushTexture(void * Data, u32 DataSize, u32 Width, u32 Height, u32 BaseOffset);
}

enum enum_textures
{
    texture_ground_stone = 1
};

void
GetTexture(game_state * GameState,game_memory * Memory,memory_arena * Arena, enum_textures TextureID)
{
    /*
     *
     * Reading a 2MB jpeg requires about 10 MB of temporary memory to do the entire process
     * without cleaning up in between
     */
    Assert(TextureID == 1);
    const char * PathTextures = {
        "..\\..\\assets\\ground_stone_01.jpg"
    };

    file_contents GetFileResult = GetFileContents(Memory, Arena,&PathTextures[TextureID - 1]);
    if (GetFileResult.Success)
    {
        i32 x,y,comp;
        stbi_uc * Data = stbi_load_from_memory(Arena,GetFileResult.Base, GetFileResult.Size, &x,&y, &comp, 0);
        u32 Size = x * y * comp;
        //RenderPushTexture(Data, x, y, BaseOffset);
    }
    else
    {
        Assert(0); // Expecting texture to be loaded
    }
}

void
DestroyPipelines(game_state * GameState,render_controller * Renderer)
{
    for (i32 i = 0; i < ArrayCount(Renderer->Pipelines); ++i)
    {
        Renderer->Pipelines[i] = -1;
    }
    RenderFreeShaders();
    VulkanDestroyPipeline();
}

void
CreateAllPipelines(game_state * GameState, game_memory * Memory)
{
    for (i32 i = 0; i < material_type_count; ++i)
    {
        pipeline_creation_result Result =  CreatePipeline(Memory,&GameState->TemporaryArena,&GameState->Renderer,material_type_texture);
        GameState->Renderer.Pipelines[i] = Result.Pipeline;
    }
}

pipeline_creation_result
CreatePipeline(game_memory * Memory,memory_arena * TempArena,render_controller * Renderer,material_type Material)
{
    Assert((i32)Material < ArrayCount(Renderer->Pipelines));
    const char * ShadersVertexFile[] = {
        "shaders\\triangle.vert",
        "shaders\\triangle_text.vert",
    };
    const char * ShadersFragmentFile[] = {
        "shaders\\triangle.frag",
        "shaders\\triangle_text.frag",
    };

    BeginTempArena(TempArena,1);

    Renderer->VertexShaders[Material] = LoadShader(Memory,TempArena,ShadersVertexFile[Material]);
    Renderer->FragmentShaders[Material] = LoadShader(Memory,TempArena,ShadersFragmentFile[Material]);

    pipeline_creation_result PipelineResult = RenderCreatePipeline(Renderer->VertexShaders[Material], Renderer->FragmentShaders[Material]);

    // TODO: destroy shaders after pipeline creation
    Assert(PipelineResult.Success);

    Renderer->Pipelines[Material] = PipelineResult.Pipeline;

    EndTempArena(TempArena,1);

    return PipelineResult;
}
