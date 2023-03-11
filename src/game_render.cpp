#include "game.h"

inline v3
GetViewPos(render_controller * Renderer)
{
    // Render negates view position.
    v3 P = -Renderer->ViewMoveMatrix[3].xyz;
    
    return P;
}

m4
OrthographicProjectionMatrix(r32 r, r32 l, r32 t, r32 b, r32 f, r32 n)
{
    m4 m = {};

#if 0
    m[0].x = 2.0f * (1.0f / (r - l));
    m[1].y = 2.0f * (1.0f / (t - b));
    m[2].z = -2.0f * (1.0f / (f - n));
    m[0].w = -(r + l) / (r - l);
    m[1].w = -(t + b) / (t - b);
    m[2].w = -(f + n) / (f - n);
    m[3].w = 1.0f;
#else
    m[0].x = 1.0f;
    m[1].y = t;
    m[2].z = 2.0f * (1.0f / (n - f));
    m[0].w = 0;
    m[1].w = 0;
    m[2].w = (n + f) / (n - f);
    m[3].w = 1.0f;
#endif

    return m;
} 


void
UpdateRenderViewport(render_controller * Renderer,i32 ScreenWidth, i32 ScreenHeight)
{
    r32 AspectRatio = (r32)ScreenWidth / (r32)ScreenHeight;
    Renderer->ScreenWidth = ScreenWidth;
    Renderer->ScreenHeight = ScreenHeight;
    Renderer->WidthOverHeight = AspectRatio;
    Renderer->OneOverWidthOverHeight = 1.0f / AspectRatio;
}

render_controller
NewRenderController(memory_arena * Arena,
                    u32 RenderUnitLimits,
                    v3 WorldUp,
                    r32 FOV,
                    i32 ScreenWidth, i32 ScreenHeight, 
                    r32 n, r32 f,
                    v3 StartP,
                    projection_mode ProjectionMode)
{
    render_controller Renderer;

    Renderer.WorldUp            = WorldUp; // RECORD   WorldUp;

    Translate(Renderer.ViewMoveMatrix, -StartP);

    RotateFill(&Renderer.ViewRotationMatrix, 0, 0, 0);
    Renderer.ViewRotationMatrix = M4();
    Renderer.ViewTransform      = M4();

    UpdateRenderViewport(&Renderer,ScreenWidth, ScreenHeight);

    switch (ProjectionMode)
    {

        case projection_perspective:
        {
            Renderer.Projection = ProjectionMatrix(FOV,Renderer.WidthOverHeight, n,f);
        } break;
        case projection_orthographic:
        {
            r32 r = 1.0f;
            r32 l = 0.0f;
            r32 t = r * Renderer.WidthOverHeight;
            r32 b = 0.0f;
            Renderer.Projection = OrthographicProjectionMatrix(r,l,t,b,100.0f,0.1f);
        } break;
    }

    // Expects power of 2
    u32 RenderUnitLimitsPerQueue = RenderUnitLimits >> 1;

    Renderer.UnitsOpaque.UnitsLimit = RenderUnitLimitsPerQueue;
    Renderer.UnitsOpaque.Units = PushArray(Arena,RenderUnitLimitsPerQueue,render_unit);
    Renderer.UnitsOpaque.UnitsCount = 0;

    Renderer.UnitsTransparent.UnitsLimit = RenderUnitLimitsPerQueue;
    Renderer.UnitsTransparent.Units = PushArray(Arena,RenderUnitLimitsPerQueue,render_unit);
    Renderer.UnitsTransparent.UnitsCount = 0;

#if DEBUG
    b32 WaitUntilLoaded = true;
    asset_material * WireframePipeline = 
        GetMaterial(GlobalAssets,game_asset_material_default_light_wireframe, WaitUntilLoaded);
    Assert(WireframePipeline);
    Assert(WireframePipeline->Pipeline[0].Success);
    Renderer.DebugWireframeMaterialPipelineIndex =  WireframePipeline->Pipeline[0].Pipeline;

    Renderer.UnitsBBV.UnitsLimit = RenderUnitLimitsPerQueue;
    Renderer.UnitsBBV.Units = PushArray(Arena,RenderUnitLimitsPerQueue,render_unit);
    Renderer.UnitsBBV.UnitsCount = 0;
#endif

    return Renderer;
}

void
UpdateView(render_controller * Renderer)
{
    Renderer->ViewTransform = Renderer->ViewRotationMatrix * Renderer->ViewMoveMatrix;
}


#if 0
void
RenderDrawGround(game_state * GameState,render_controller * Renderer, simulation * Sim)
{
    simulation_iterator SimIter = BeginSimGroundIterator(&GameState->World, Sim);

    u32 PipelineIndex = 1;

    RenderSetPipeline(Renderer->Pipelines[PipelineIndex]);

    RenderBindMaterial(PipelineIndex);

    u32 ObjectCount,BeginObjectCount;
    GPUObjectData * ObjectData = VulkanBeginObjectDataMapping(&ObjectCount);
    BeginObjectCount = ObjectCount;

    for (entity * Entity = SimIter.Entity;
            Entity;
            Entity = AdvanceSimGroundIterator(&SimIter))
    {
        m4 ModelTransform = Entity->Transform.WorldT;

        m4 MVP = Renderer->Projection * Renderer->ViewTransform * ModelTransform;

        ObjectData->ModelMatrix = ModelTransform;
        ObjectData->MVP = MVP;
        ObjectData->Color = V4(Entity->Color,1.0f - Entity->Transparency);

        ++ObjectCount;
        ++ObjectData;
    }

    VulkanEndObjectDataMapping(ObjectCount);

    ObjectCount = BeginObjectCount;
    SimIter = BeginSimGroundIterator(&GameState->World, Sim);

    for (entity * Entity = SimIter.Entity;
            Entity;
            Entity = AdvanceSimGroundIterator(&SimIter))
    {
        mesh_group * MeshGroup = GameState->GroundMeshGroup + Entity->MeshID.ID;
        mesh * Mesh = MeshGroup->Meshes;
#if 0
            mesh_push_constant Constants;
            Constants.RenderMatrix = MVP;
            Constants.Model = ModelTransform;
            Constants.DebugColor = V4(Entity->Color,1.0f - Entity->Transparency);
            RenderPushVertexConstant(sizeof(mesh_push_constant),(void *)&Constants);
#endif

        u32 MeshSize = Mesh->VertexSize /sizeof(vertex_point);
        RenderBindMesh(MeshSize, Mesh->OffsetVertices);
        RenderDrawObject(MeshSize,ObjectCount);

        ++ObjectCount;
    }

}
#endif

#if DEBUG
#define PushDrawBBV PushDrawBBV_
void
PushDrawBBV_(render_controller * Renderer, m4 * ModelT)
{
    render_unit * Unit = 0;
    Assert(Renderer->UnitsBBV.UnitsCount <= Renderer->UnitsBBV.UnitsLimit);
    Unit = Renderer->UnitsBBV.Units + Renderer->UnitsBBV.UnitsCount++;

    Unit->ModelTransform = *ModelT;
}
#else
#define PushDrawBBV
#endif

void
PushDraw_(render_controller * Renderer, asset_material * Material, m4 * ModelT, mesh_group * MeshGroup, asset_texture * Texture, game_asset_id TextureID, v3 Color, r32 Transparency)
{
    if (MeshGroup && Material && (Texture || TextureID == ASSETS_NULL_TEXTURE))
    {
        render_unit * Unit = 0;
        if (Material->AssetID == game_asset_material_transparent)
        {
            Assert(Renderer->UnitsTransparent.UnitsCount <= Renderer->UnitsTransparent.UnitsLimit);
            Unit = Renderer->UnitsTransparent.Units + Renderer->UnitsTransparent.UnitsCount++;
        }
        else
        {
            Assert(Renderer->UnitsOpaque.UnitsCount <= Renderer->UnitsOpaque.UnitsLimit);
            Unit = Renderer->UnitsOpaque.Units + Renderer->UnitsOpaque.UnitsCount++;
        }

        Unit->ModelTransform = *ModelT;
        Unit->MeshGroup = MeshGroup;

        for (u32 MaterialIndex = 0; 
                 MaterialIndex < Material->PipelinesCount;
                 ++MaterialIndex)
        {
            Unit->MaterialPipelineIndex[MaterialIndex] = 
                Material->Pipeline[MaterialIndex].Pipeline;
        }
        Unit->Color = V4(Color, 1.0f - Transparency);
        Unit->TextureID = Texture ? Texture->GPUID : -1;
    }
}

void
PushDrawParticle(render_controller * Renderer, m4 * ModelT, mesh_group * MeshGroup, game_asset_id TextureID, v3 Color, r32 Transparency)
{
    asset_material * MaterialPipeline = GetMaterial(GlobalAssets,game_asset_material_transparent);
    asset_texture * Texture = 0;
    if (TextureID > game_asset_texture_begin && TextureID < game_asset_texture_end)
    {
        Texture = GetTexture(GlobalAssets,TextureID);
    }
#if 0
    Texture = 0; TextureID = ASSETS_NULL_TEXTURE;
    MaterialPipeline = GetMaterial(GlobalAssets,game_asset_material_default_no_light);
#endif
    PushDraw_(Renderer, MaterialPipeline, ModelT, MeshGroup, Texture, TextureID, Color, Transparency);
}

void
PushDraw(render_controller * Renderer, game_asset_id Material, m4 * ModelT, game_asset_id MeshID, game_asset_id TextureID, v3 Color, r32 Transparency)
{


    mesh_group * MeshGroup = GetMesh(GlobalAssets,MeshID);
    asset_material * MaterialPipeline = GetMaterial(GlobalAssets,Material);
    asset_texture * Texture = 0;
    if (TextureID > game_asset_texture_begin && TextureID < game_asset_texture_end)
    {
        Texture = GetTexture(GlobalAssets,TextureID);
    }
    PushDraw_(Renderer, MaterialPipeline, ModelT, MeshGroup, Texture, TextureID,Color, Transparency);

}


void
PushDrawEntity(render_controller * Renderer,entity * Entity)
{
    PushDraw(Renderer, Entity->Material, &Entity->Transform.WorldT, Entity->MeshID, Entity->TextureID ,Entity->Color, Entity->Transparency);
}

#if DEBUG
void
PushDrawDebug(render_controller * Renderer,v3 LocalP)
{
    entity Entity= {};
    PushDrawEntity(Renderer,&Entity);
}
#endif


void
PushDrawSimulation(render_controller * Renderer,world * World, simulation * Sim)
{

    v4 Color = V4(1.0f,0.5f,0.2f,1.0f);

    simulation_iterator SimIter = BeginSimIterator(World, Sim);

    for (entity * Entity = SimIter.Entity;
            Entity;
            Entity = AdvanceSimIterator(&SimIter))
    {
        PushDrawEntity(Renderer,Entity);
#if 0
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
                PushDrawEntity(Renderer,Entity);
            }
        }
#endif
    }

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

#if 0
i32
LookAt(Quaternion * Q, v3 P, v3 TargetP, v3 WorldUp)
{
    v3 Out;
    Quaternion Q1,Q2;

    Quaternion_setIdentity(&Q1);
    Quaternion_setIdentity(&Q2);

    Out = TargetP - P;

    r32 LengthOut = Length(Out);

    if (LengthOut < 0.000001)
    {
        Quaternion_setIdentity(Q);
        return 1;
    }

    Out = Out / LengthOut;

    v3 FwAxis = V3(0,0,1.0f);
    v3 RotAxis = Cross(Out,FwAxis);

    if (LengthSqr(RotAxis)== 0)
    {
        RotAxis = WorldUp;
    }
    r32 Angle = acosf(Inner(Out,FwAxis));

    Quaternion_fromAxisAngle(RotAxis._V, Angle, Q1);

    return 0;
}
#else
i32
LookAt(Quaternion * Q, v3 P, v3 TargetP, v3 WorldUp)
{
    v3 Right, Up, Out;

    Out = TargetP - P;

    r32 LengthOut = Length(Out);

    if (LengthOut < 0.000001)
    {
        return -1;
    }

    Out = Out / LengthOut;

    Quaternion Q1,Q2;

    Quaternion_setIdentity(&Q1);
    Quaternion_setIdentity(&Q2);

    v3 FwAxis = V3(0,0,1.0f);
    v3 RotAxis = Cross(Out,FwAxis);

    if (LengthSqr(RotAxis)== 0)
    {
        RotAxis = WorldUp;
    }

    r32 Angle = acosf(Inner(Out,FwAxis));

    Quaternion_fromAxisAngle(RotAxis._V, Angle, &Q1);

    Quaternion_toAxisAngle(&Q1, Up._V);

#if 0
    Up = WorldUp - (Inner(WorldUp, Out)*Out);
    r32 LengthSqrUp = LengthSqr(Up);
    // too close
    Assert(LengthSqrUp > 0.000001f);
    Up = Up / sqrtf(LengthSqrUp);
#else
    Right = Normalize(Cross(Out, WorldUp));
    Up = Normalize(Cross(Right, Out));
#endif

    RotAxis = Cross(Up, WorldUp);
    Angle = acosf(Inner(Up,WorldUp));

    Quaternion_fromAxisAngle(RotAxis._V, Angle, &Q2);

    Quaternion_multiply(&Q2, &Q1, Q);

    return 0;
}
#endif

i32
LookAt(m4 * M, v3 P, v3 TargetP, v3 WorldUp)
{
    v3 Right, Up, Out;

    Out = TargetP - P;

    r32 LengthOut = Length(Out);

    if (LengthOut < 0.000001)
    {
        return -1;
    }

    Out = Out / LengthOut;

    Up = WorldUp - (Inner(WorldUp, Out)*Out);

    r32 LengthSqrUp = LengthSqr(Up);

    if (LengthSqrUp < 0.000001f)
    {
        return -1;
    }

    Up = Up / sqrtf(LengthSqrUp);

    Right = Cross(Out,Up);

    // TODO: this is a hack I don't understand what I'm doing
#if 1
    if (Inner(V3(0,0,1), Out) > 0.0f)
    {
        Up = -Up;
    }
#endif

    (*M)[0].x = Right.x;
    (*M)[0].y = Up.x;
    (*M)[0].z = Out.x;
    (*M)[0].w = 0;

    (*M)[1].x = Right.y;
    (*M)[1].y = Up.y;
    (*M)[1].z = Out.y;
    (*M)[1].w = 0;

    (*M)[2].x = Right.z;
    (*M)[2].y = Up.z;
    (*M)[2].z = Out.z;
    (*M)[2].w = 0;

    (*M)[3].x = 0;
    (*M)[3].y = 0;
    (*M)[3].z = 0;
    (*M)[3].w = 1;

    return 0;
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

void
BeginRender(render_controller * Renderer,i32 ScreenWidth, i32 ScreenHeight)
{
    Renderer->UnitsTransparent.UnitsCount = 0;
    Renderer->UnitsOpaque.UnitsCount = 0;
    UpdateRenderViewport(Renderer,ScreenWidth, ScreenHeight);
    UpdateView(Renderer);
}

void
BeginRenderPass(v4 ClearColor, v4 AmbientLight, v4 SunlightDirection, v4 SunlightColor)
{
    GPUSimulationData SimData = {};

    SimData.AmbientLight      = AmbientLight; // RECORD   AmbientLight;
    SimData.SunlightDirection = SunlightDirection;
    SimData.SunlightColor     = SunlightColor; // RECORD   SunlightColor;

    GraphicsBeginRenderPass(ClearColor,&SimData);
}

void
RenderDraw(render_controller * Renderer)
{
    GraphicsRenderDraw(Renderer);
}

void
EndRender()
{
    GraphicsEndRenderPass();
}


