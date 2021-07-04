#include "math.h"
#include "game.h"
#include "vulkan_initializer.h"
//#include "render_2.cpp"

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
Translate(m4 * MM,v3 P)
{
    MM->Columns[0] = {1, 0 , 0 , 0};
    MM->Columns[1] = {0, 1 , 0 , 0};
    MM->Columns[2] = {0, 0 , 1 , 0};
    MM->Columns[3] = {P.x, P.y , P.z , 1};
}
v3
RenderGetViewUp(renderer_3d * Renderer)
{
    v3 D = V3(Renderer->ViewRotationMatrix[0].y,Renderer->ViewRotationMatrix[1].y,Renderer->ViewRotationMatrix[2].y);
    
    return D;
}
v3
RenderGetViewRight(renderer_3d * Renderer)
{
    v3 D = V3(Renderer->ViewRotationMatrix[0].x,Renderer->ViewRotationMatrix[1].x,Renderer->ViewRotationMatrix[2].x);
    
    return D;
}
v3
GetObjectDirection(m4 &M)
{
    v3 D = V3(M[0].z,M[1].z,M[2].z);
    
    return D;
}
v3
RenderGetObjectDirection(renderer_3d * Renderer)
{
    v3 D = V3(Renderer->ObjectRotationMatrix[0].z,Renderer->ObjectRotationMatrix[1].z,Renderer->ObjectRotationMatrix[2].z);
    
    return D;
}
v3
RenderGetViewDirection(renderer_3d * Renderer)
{
    v3 D = V3(Renderer->ViewRotationMatrix[0].z,Renderer->ViewRotationMatrix[1].z,Renderer->ViewRotationMatrix[2].z);
    
    return D;
}
v3
RenderGetViewPos(renderer_3d * Renderer)
{
    // Render negates view position.
    v3 P = -Renderer->ViewMoveMatrix[3].xyz;
    
    return P;
}
v3
RenderGetObjectPos(renderer_3d * Renderer)
{
    // Render negates view position.
    v3 P = -Renderer->ViewMoveMatrix[3].xyz;
    
    return P;
}

void
Rotate(m4 * A,v3 Rotation)
{
    *A = M4();
}


void
RenderUpdateObject(renderer_3d * Renderer)
{
    Renderer->ObjectTransform =  Renderer->ObjectMoveMatrix * Renderer->ObjectRotationMatrix * Renderer->ObjectScale;
    Renderer->MVP = Renderer->Projection * Renderer->WorldTransform * Renderer->ObjectTransform;
}
void
RenderUpdateView(renderer_3d * Renderer)
{
#if 0
    Renderer->ViewRotationMatrix[0].z = 1.0f;
    Renderer->ViewRotationMatrix[1].z = 1.0f;
    Renderer->ViewRotationMatrix[2].z = 1.0f;
    Renderer->ViewRotationMatrix[3].z = 0;
#endif

    Renderer->WorldTransform = Renderer->ViewRotationMatrix * Renderer->ViewMoveMatrix;
}
void
RenderRotateView(renderer_3d * Renderer, real32 Pitch, real32 Yaw)
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

    Renderer->ViewRotationMatrix = R * Renderer->ViewRotationMatrix;

    RenderUpdateView(Renderer);
}
void
RotateObjectUp(m4 * M,real32 Angle)
{

    real32 c = cosf(-Angle);
    real32 s = sinf(-Angle);
    m4 R = {
        1, 0 , 0 , 0,
        0, c, -s , 0,
        0, s , c, 0,
        0, 0, 0, 1
    };

    *M = R * (*M);
}

void
RotateObjectRight(m4 * M,real32 Angle)
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
RenderRotateObjectRight(renderer_3d * Renderer,real32 Angle)
{
    real32 c = cosf(-Angle);
    real32 s = sinf(-Angle);
    m4 R = {
        c, 0 , s , 0,
        0, 1, 0 , 0,
        -s, 0 , c, 0,
        0, 0, 0, 1
    };

    Renderer->ObjectRotationMatrix = R * Renderer->ObjectRotationMatrix;
    RenderUpdateObject(Renderer);
}
void
RenderRotateViewRight(renderer_3d * Renderer,real32 Angle)
{

    real32 c = cosf(-Angle);
    real32 s = sinf(-Angle);
    m4 R = {
        c, 0 , s , 0,
        0, 1, 0 , 0,
        -s, 0 , c, 0,
        0, 0, 0, 1
    };

    Renderer->ViewRotationMatrix = R * Renderer->ViewRotationMatrix;
    RenderUpdateView(Renderer);
}
void
RenderRotateViewUp(renderer_3d * Renderer,real32 Angle)
{

    real32 c = cosf(-Angle);
    real32 s = sinf(-Angle);
    m4 R = {
        1, 0 , 0 , 0,
        0, c, -s , 0,
        0, s , c, 0,
        0, 0, 0, 1
    };

    Renderer->ViewRotationMatrix = R * Renderer->ViewRotationMatrix;
    RenderUpdateView(Renderer);
}
void
RenderMoveViewForward(renderer_3d * Renderer,real32 N)
{
    v3 P = RenderGetViewPos(Renderer);
    v3 Out = RenderGetViewDirection(Renderer);
    // do not alter Y
    //Out.y = 0.0f;
    v3 R = P + (N * Out);
    Translate(&Renderer->ViewMoveMatrix, -R);
    RenderUpdateView(Renderer);
}
void
RenderMoveObjectForward(renderer_3d * Renderer,real32 N)
{
    v3 P = RenderGetObjectPos(Renderer);
    v3 Out = RenderGetObjectDirection(Renderer);
    // do not alter Y
    //Out.y = 0.0f;
    P = P + (N * Out);
    Translate(&Renderer->ObjectMoveMatrix, P);
    RenderUpdateObject(Renderer);
}
void
RenderMoveViewRight(renderer_3d * Renderer,real32 N)
{
    v3 P = RenderGetViewPos(Renderer);
    v3 Right = RenderGetViewRight(Renderer);
    // do not alter Y
    Right.y = 0.0f;
    v3 R = P + (N * Right);
    Translate(&Renderer->ViewMoveMatrix, -R);
    RenderUpdateView(Renderer);
}


void
RenderMoveView(renderer_3d * Renderer,v3 AbsP)
{
    Translate(&Renderer->ViewMoveMatrix , -AbsP);
    Renderer->WorldTransform = Renderer->ViewRotationMatrix * Renderer->ViewMoveMatrix;
}


void
RenderRotateFill(m4 * M, real32 AngleX, real32 AngleY, real32 AngleZ)
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

    *M = AxisZ * AxisY * AxisX;
}

m4
LookAtFromD(v3 D, v3 WorldUp)
{
    v3 Up = WorldUp - (Inner(WorldUp,D)*D);
    v3 Right = Cross(D,Up);

    m4 R = {};
    R[0].x = Right.x;
    R[1].x = Up.x;
    R[2].x = D.x;
    R[3].x = 0;

    R[0].y = Right.y;
    R[1].y = Up.y;
    R[2].y = D.y;
    R[3].y = 0;

    R[0].z = Right.z;
    R[1].z = Up.z;
    R[2].z = D.z;
    R[3].z = 0;

    R[0].w = 0;
    R[1].w = 0;
    R[2].w = 0;
    R[3].w = 1;

    return R;
}
m4
LookAt(v3 Position, v3 Target, v3 WorldUp)
{
    v3 D = (Target - Position) / Length(Target - Position);
    v3 Up = WorldUp - (Inner(WorldUp,D)*D);
    v3 Right = Cross(D,Up);

    m4 R = {};
    R[0].x = Right.x;
    R[1].x = Up.x;
    R[2].x = D.x;
    R[3].x = 0;

    R[0].y = Right.y;
    R[1].y = Up.y;
    R[2].y = D.y;
    R[3].y = 0;

    R[0].z = Right.z;
    R[1].z = Up.z;
    R[2].z = D.z;
    R[3].z = 0;

    R[0].w = 0;
    R[1].w = 0;
    R[2].w = 0;
    R[3].w = 1;

    return R;
}

void
RenderSetObjectRotationFromDirection(renderer_3d * Renderer, v3 D)
{
    D = D / Length(D);
#if 1
    v3 Up = Renderer->WorldUp - (Inner(Renderer->WorldUp,D)*D);
    Up = Up / Length(Up);
#else
    v3 Up = Renderer->WorldUp;
#endif
#if 1
    v3 Right = Cross(D,Up);
#else

    v3 Right = V3(1,0,0);
#endif

    Renderer->ObjectRotationMatrix[0].x = Right.x;
    Renderer->ObjectRotationMatrix[1].x = Up.x;
    Renderer->ObjectRotationMatrix[2].x = D.x;
    Renderer->ObjectRotationMatrix[3].x = 0;

    Renderer->ObjectRotationMatrix[0].y = Right.y;
    Renderer->ObjectRotationMatrix[1].y = Up.y;
    Renderer->ObjectRotationMatrix[2].y = D.y;
    Renderer->ObjectRotationMatrix[3].y = 0;

    Renderer->ObjectRotationMatrix[0].z = Right.z;
    Renderer->ObjectRotationMatrix[1].z = Up.z;
    Renderer->ObjectRotationMatrix[2].z = D.z;
    Renderer->ObjectRotationMatrix[3].z = 0;

    Renderer->ObjectRotationMatrix[0].w = 0;
    Renderer->ObjectRotationMatrix[1].w = 0;
    Renderer->ObjectRotationMatrix[2].w = 0;
    Renderer->ObjectRotationMatrix[3].w = 1;
}

void
SetObject(renderer_3d * Renderer, v3 P , v3 D, v3 Scale = {1,1,1})
{
    Renderer->ObjectScale = M4Scale(Scale);
    Translate(&Renderer->ObjectMoveMatrix, P);
    RenderSetObjectRotationFromDirection(Renderer,D);

    RenderUpdateObject(Renderer);
}

int32
RenderLookAt(renderer_3d * Renderer,v3 P, v3 TargetP)
{
    v3 Right, Up, Out;

    Out = P - TargetP;

    real32 LengthOut = Length(Out);

    if (LengthOut < 0.000001)
    {
        return -1;
    }

    Out = Out / LengthOut;

    Up = Renderer->WorldUp - (Inner(Renderer->WorldUp, Out)*Out);

    real32 LengthSqrUp = LengthSqr(Up);

    // too close
    Assert(LengthSqrUp > 0.000001f);

    Up = Up / sqrtf(LengthSqrUp);

    //Right = Cross(Out,Up);
    Right = Cross(Up,Out);

    Renderer->ViewRotationMatrix[0].x = Right.x;
    Renderer->ViewRotationMatrix[0].y = Up.x;
    Renderer->ViewRotationMatrix[0].z = Out.x;
    Renderer->ViewRotationMatrix[0].w = 0;

    Renderer->ViewRotationMatrix[1].x = Right.y;
    Renderer->ViewRotationMatrix[1].y = Up.y;
    Renderer->ViewRotationMatrix[1].z = Out.y;
    Renderer->ViewRotationMatrix[1].w = 0;

    Renderer->ViewRotationMatrix[2].x = Right.z;
    Renderer->ViewRotationMatrix[2].y = Up.z;
    Renderer->ViewRotationMatrix[2].z = Out.z;
    Renderer->ViewRotationMatrix[2].w = 0;

    Renderer->ViewRotationMatrix[3].x = 0;
    Renderer->ViewRotationMatrix[3].y = 0;
    Renderer->ViewRotationMatrix[3].z = 0;
    Renderer->ViewRotationMatrix[3].w = 1;

    Translate(&Renderer->ViewMoveMatrix, -P);

    Renderer->WorldTransform = Renderer->ViewRotationMatrix * Renderer->ViewMoveMatrix;

    return 0;
}
void
SetViewBehindObject(renderer_3d * Renderer, v3 T, v3 D, real32 Separation, real32 HeightOverObject = 0.0f)
{
    D = (D / Length(D));

    v3 V = T - D * Separation + V3(0,HeightOverObject,0);

    RenderLookAt(Renderer,V,T);
}

void
BeginRender(game_state * GameState, v4 ClearColor)
{
    WaitForRender();

    RenderBeginPass(ClearColor);

    RenderSetPipeline(GameState->PipelineIndex);
}

renderer_3d
CreateRenderer(real32 FOV,
               int32 ScreenWidth, int32 ScreenHeight, 
               real32 n, real32 f, 
               v3 P, v3 WorldUp = DEFAULT_WORLD_UP)
{
    renderer_3d Renderer = {};

    Renderer.WorldUp = WorldUp;
    Renderer.Projection = ProjectionMatrix(FOV,((real32)ScreenWidth / (real32)ScreenHeight), n,f);

    // Depends on world up
    Translate(&Renderer.ViewMoveMatrix,-P);
    RenderRotateFill(&Renderer.ViewRotationMatrix, 0, 0, 0);

    // Depends on LookAt and Projection matrices
    SetObject(&Renderer, V3(0,0,0), V3(0,0,0));

    return Renderer;
}

void
EndRender(game_state * GameState)
{
    RenderEndPass();
}

void
RenderMesh(renderer_3d * Renderer, entity * Entity, v3 Rotation, v4 Color, v3 SourceLight)
{
    mesh * Mesh = Entity->Mesh;
    Assert(Mesh);

    SetObject(Renderer, Entity->P, Entity->D, Entity->Scale);

    if (Rotation.y)
    {
        RenderRotateObjectRight(Renderer, Rotation.y);
    }

    Entity->D = RenderGetObjectDirection(Renderer);

    mesh_push_constant Constants;

    Constants.RenderMatrix = Renderer->MVP;
    Constants.ViewRotationMatrix = Renderer->ObjectRotationMatrix;
    Constants.SourceLight = SourceLight;
    Constants.Model = Renderer->ObjectTransform;
    //Constants.Model = Renderer->WorldTransform * Renderer->ObjectTransform;
    Constants.IsLightSource = (SourceLight.y == 0.0f);
    Constants.DebugColor = Color;

    RenderPushVertexConstant(sizeof(mesh_push_constant),(void *)&Constants);
    //RenderPushMesh(1,(Mesh->IndicesSize / sizeof(uint16)),Mesh->OffsetVertices,Mesh->OffsetIndices);
    RenderPushMesh(1, Mesh->VertexSize / sizeof(vertex_point));
            
}
void
RenderMeshAround(renderer_3d * Renderer, entity * Entity, v3 TargetP, v3 TargetD, real32 Radius, v3 Rotation, v4 Color, v3 SourceLight)
{
    mesh * Mesh = Entity->Mesh;
    Assert(Mesh);

    m4 V = M4();
    Translate(&V, V3(Radius,0,0));
    m4 R = LookAtFromD(V3(0,0,-1), V3(0,1,0));
    RotateObjectRight(&R, Rotation.x);
    RotateObjectUp(&R,Rotation.y);
    //RenderRotateFill(&R, Rotation.x, Rotation.y, Rotation.z);

    m4 M;
    Translate(&M, TargetP);
    V = M * (R * V);

    m4 MVP = Renderer->Projection * Renderer->WorldTransform * V;

    mesh_push_constant Constants;

    Constants.DebugColor = Color;
    Constants.ViewRotationMatrix = Renderer->ObjectRotationMatrix;
    //Constants.Model = Renderer->WorldTransform * V;
    Constants.Model = V;
    Constants.SourceLight = SourceLight;
    Constants.IsLightSource = (SourceLight.y == 0.0f);
    Constants.RenderMatrix = MVP;

    RenderPushVertexConstant(sizeof(mesh_push_constant),(void *)&Constants);
    //RenderPushMesh(1,(Mesh->IndicesSize / sizeof(uint16)),Mesh->OffsetVertices,Mesh->OffsetIndices);
    RenderPushMesh(1, Mesh->VertexSize / sizeof(vertex_point));
}
