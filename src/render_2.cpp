#include "math.h"
#include "game.h"
#include "vulkan_initializer.h"



inline void Translate(m4 &M,v3 P);
void RotateFill(m4 * M, real32 AngleX, real32 AngleY, real32 AngleZ);

inline v3
GetViewRight(m4 &ViewRotationMatrix)
{
    v3 D = V3(ViewRotationMatrix[0].x,ViewRotationMatrix[1].x,ViewRotationMatrix[2].x);
    
    return D;
}inline v3
GetEntityRight(m4 &RotationMatrix)
{
    v3 D = V3(RotationMatrix[0].x,RotationMatrix[1].x,RotationMatrix[2].x);
    
    return D;
}
inline v3
GetEntityPos(m4 &M)
{
    // Render negates view position.
    v3 P = M[3].xyz;
    
    return P;
}
inline v3
GetViewPos(m4 &View)
{
    // Render negates view position.
    v3 P = -View[3].xyz;
    
    return P;
}
inline v3
GetViewDirection(m4 &ViewRotationMatrix)
{
    v3 D = V3(ViewRotationMatrix[0].z,ViewRotationMatrix[1].z,ViewRotationMatrix[2].z);
    
    return D;
}
inline v3
GetEntityDirection(m4 &RotationMatrix)
{
    v3 D = V3(RotationMatrix[0].z,RotationMatrix[1].z,RotationMatrix[2].z);
    
    return D;
}

m4
ProjectionMatrix(real32 FOV,real32 AspectRatio, real32 n, real32 f)
{
    m4 m = {};

    real32 HalfTanFOV = (real32)(tan(FOV * 0.5f));
    real32 A = -(f + n) / (f - n);
    real32 B = (-2*f*n) / (f - n);

    m[0].x = 1.0f / (HalfTanFOV * AspectRatio); // (2 * n) / (r - l);
    // m.c20 = 0; //(r + l) / (r - l);
    m[1].y = 1.0f / (HalfTanFOV); // (2 * n) / (t - b);
    // m.c21 = 0; //(t + b) / (t - b);
    m[2].z = A;
    m[3].z = B;
    m[2].w = -1; // -Pz
    //m.c33 = 0;  // w

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

mesh *
GetMesh(game_state * GameState,uint32 ID)
{
    mesh * Mesh = (GameState->Meshes + ID);
    return Mesh;
}

void
UpdateView(game_state * GameState)
{
    GameState->ViewTransform = GameState->ViewRotationMatrix * GameState->ViewMoveMatrix;
}

void
RenderEntities(game_state * GameState)
{
    v3 SourceLight = V3(0,10.0f,0);
    v4 Color = V4(1.0f,0.5f,0.2f,1.0f);

    UpdateView(GameState);

    v3 ViewPos = GetViewPos(GameState->ViewMoveMatrix);
    v3 ViewDirection = -GetViewDirection(GameState->ViewRotationMatrix);

    for (uint32 EntityIndex = 0;
                EntityIndex < GameState->TotalEntities;
                ++EntityIndex)
    {
        uint32 EntityID = GameState->Entities[EntityIndex].ID;
        entity_transform * T = (GameState->EntitiesTransform + EntityID);

        v3 ViewToEntity = GetEntityPos(T->WorldP) - ViewPos;
        real32 ViewProjectionOntoEntity = Inner(ViewToEntity,ViewDirection); 

        // We are culling entities behind camera
        // TODO: Fix ground as it will be behind camera and dissapear
        if (ViewProjectionOntoEntity >= 0.0f)
        {
            render_3D * R3D = GameState->Render3D + EntityID;
            mesh * Mesh = GetMesh(GameState,R3D->MeshID);
            m4 ModelTransform = T->WorldT;

            mesh_push_constant Constants;
            m4 MVP = GameState->Projection * GameState->ViewTransform * ModelTransform;

            Constants.RenderMatrix = MVP;
            Constants.SourceLight = SourceLight;
            Constants.Model = ModelTransform;
            Constants.DebugColor = Color;

            RenderPushVertexConstant(sizeof(mesh_push_constant),(void *)&Constants);
            //RenderPushMesh(1,(Mesh->IndicesSize / sizeof(uint16)),Mesh->OffsetVertices,Mesh->OffsetIndices);
            RenderPushMesh(1, Mesh->VertexSize / sizeof(vertex_point));
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


int32
ViewLookAt(game_state * GameState, v3 P, v3 TargetP)
{
    v3 Right, Up, Out;

    Out = P - TargetP;

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

    //Right = Cross(Out,Up);
    Right = Cross(Up,Out);

    GameState->ViewRotationMatrix[0].x = Right.x;
    GameState->ViewRotationMatrix[0].y = Up.x;
    GameState->ViewRotationMatrix[0].z = Out.x;
    GameState->ViewRotationMatrix[0].w = 0;

    GameState->ViewRotationMatrix[1].x = Right.y;
    GameState->ViewRotationMatrix[1].y = Up.y;
    GameState->ViewRotationMatrix[1].z = Out.y;
    GameState->ViewRotationMatrix[1].w = 0;

    GameState->ViewRotationMatrix[2].x = Right.z;
    GameState->ViewRotationMatrix[2].y = Up.z;
    GameState->ViewRotationMatrix[2].z = Out.z;
    GameState->ViewRotationMatrix[2].w = 0;

    GameState->ViewRotationMatrix[3].x = 0;
    GameState->ViewRotationMatrix[3].y = 0;
    GameState->ViewRotationMatrix[3].z = 0;
    GameState->ViewRotationMatrix[3].w = 1;

    // opposite to the scene entities
    Translate(GameState->ViewMoveMatrix, -P);

    UpdateView(GameState);

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

void
MoveViewRight(game_state * GameState,real32 N)
{
    v3 P = GetViewPos(GameState->ViewMoveMatrix);
    v3 Right = GetViewRight(GameState->ViewRotationMatrix);
    // do not alter Y
    //Right.y = 0.0f;
    v3 R = P + (N * Right);
    Translate(GameState->ViewMoveMatrix, -R);
    UpdateView(GameState);
}
void
MoveViewForward(game_state * GameState,real32 N)
{
    v3 P = GetViewPos(GameState->ViewMoveMatrix);
    v3 Out = GetViewDirection(GameState->ViewRotationMatrix);
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
