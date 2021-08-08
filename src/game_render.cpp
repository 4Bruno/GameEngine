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
WorldInitializeView(game_state * GameState,
               real32 FOV,
               int32 ScreenWidth, int32 ScreenHeight, 
               real32 n, real32 f, 
               v3 P, v3 WorldUp)
{
    GameState->WorldUp = WorldUp;
    GameState->Projection = ProjectionMatrix(FOV,((real32)ScreenWidth / (real32)ScreenHeight), n,f);

    // Depends on world up
    Translate(GameState->ViewMoveMatrix,-P);

    RotateFill(&GameState->ViewRotationMatrix, 0, 0, 0);
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
