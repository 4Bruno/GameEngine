#include "math.h"
#include "game.h"
#include "vulkan_initializer.h"
//#include "render_2.cpp"



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
RotateObject(v3 * D, real32 Yaw, real32 Pitch)
{
    m4 R;
    RenderRotateFill(&R, Pitch , Yaw ,0);
    *D = GetObjectDirection(R);
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
RenderMoveView(renderer_3d * Renderer,v3 AbsP)
{
    Translate(&Renderer->ViewMoveMatrix , -AbsP);
    Renderer->WorldTransform = Renderer->ViewRotationMatrix * Renderer->ViewMoveMatrix;
}

void
RenderRotateFill2(m4 * M, real32 AngleX, real32 AngleY)
{ 
    real32 cosx = cosf(AngleX);
    real32 sinx = sinf(AngleX);
    real32 cosy = cosf(AngleY);
    real32 siny = sinf(AngleY);

    v3 xaxis = { cosy, 0, -siny };
    v3 yaxis = { siny * sinx, cosx, cosy * sinx };
    v3 zaxis = { siny * cosx, -sinx, cosx * cosy };

    // Create a 4x4 view matrix from the right, up, forward and eye position vectors
    m4 ViewMatrix = {
        V4(       xaxis.x,            yaxis.x,            zaxis.x,      0 ),
        V4(       xaxis.y,            yaxis.y,            zaxis.y,      0 ),
        V4(       xaxis.z,            yaxis.z,            zaxis.z,      0 ),
        0,0,0,1
    };

    *M = ViewMatrix;
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

void
SetViewBehindObject(renderer_3d * Renderer, v3 T, v3 D, real32 Separation, real32 HeightOverObject = 0.0f)
{
    D = (D / Length(D));

    v3 V = T - D * Separation + V3(0,HeightOverObject,0);

    RenderLookAt(Renderer,V,T);
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
RenderMesh(renderer_3d * Renderer, entity * Entity, v3 Rotation, v4 Color, v3 SourceLight)
{
    mesh * Mesh = Entity->Mesh;
    Assert(Mesh);

    SetObject(Renderer, Entity->P, Entity->D, Entity->Scale);

    RenderRotateFill(&Renderer->ObjectRotationMatrix, -ToRadians(Entity->Pitch), -ToRadians(Entity->Yaw), 0.0f);

    Entity->D = RenderGetObjectDirection(Renderer);

            
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
