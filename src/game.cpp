#include "game.h"
#include "math.h"
#include "model_loader.cpp"
#include "render.cpp"


enum  shader_type_vertex
{
    shader_type_vertex_default,


    shader_type_vertex_total
};

enum  shader_type_fragment
{
    shader_type_fragment_default,


    shader_type_fragment_total
};

struct file_contents
{
    uint8 * Base;
    uint32 Size;
    bool32 Success;
};

file_contents
GetFileContents(game_memory * GameMemory,memory_arena * Arena,const char * Filepath)
{
    file_contents Result = {};
    void * Buffer = 0;

    platform_open_file_result OpenFileResult = GameMemory->DebugOpenFile(Filepath);

    if (OpenFileResult.Success)
    {
        Buffer = PushSize(Arena,OpenFileResult.Size);
        if (GameMemory->DebugReadFile(OpenFileResult,Buffer))
        {
            Result.Size = OpenFileResult.Size;
            Result.Base = (uint8 *)Buffer;
            Result.Success = true;
        }
        else
        {
            Arena->CurrentSize -= OpenFileResult.Size;
        }
        GameMemory->DebugCloseFile(OpenFileResult);
    }
    return Result;
}

void
InitializeArena(memory_arena * Arena,uint8 * BaseAddr, uint32 MaxSize)
{
    Arena->MaxSize = MaxSize;
    Arena->CurrentSize = 0;
    Arena->Base = BaseAddr;
}

mesh
LoadModel(game_memory * Memory,memory_arena * ArenaMeshes, memory_arena * ArenaTemp,const char * Filepath)
{
    mesh Mesh = {};
    int32 Result = -1;
    file_contents GetFileResult = GetFileContents(Memory, ArenaTemp,Filepath);
    if (GetFileResult.Success)
    {
        obj_file_header Header = ReadObjFileHeader((const char *)GetFileResult.Base, GetFileResult.Size);
        Mesh = CreateMeshFromObjHeader(ArenaMeshes,Header, (const char *)GetFileResult.Base, GetFileResult.Size);
        ArenaTemp->CurrentSize -= GetFileResult.Size;
    }

    return Mesh;
}

int32
LoadShader(game_memory * Memory,memory_arena * Arena,const char * Filepath)
{
    int32 Result = -1;
    file_contents GetFileResult = GetFileContents(Memory, Arena,Filepath);
    if (GetFileResult.Success)
    {
        Result = RenderCreateShaderModule((char *)GetFileResult.Base, (size_t)GetFileResult.Size);
        // data lives on gpu side now
        Arena->CurrentSize -= GetFileResult.Size;
    }
    return Result;
}
#if 0
mesh
CreateTriangle(memory_arena * Arena, v3 P, real32 SideLength)
{
    mesh Mesh = {};

    uint32 Vertices = 3;
    Mesh.VertexSize  = sizeof(vertex_point)*Vertices;
    Mesh.Vertices    = PushArray(Arena, 3, vertex_point); // vertex_point * Vertices;
    Mesh.Vertices[0].P = P;
    Mesh.Vertices[1].P = (P + V3(SideLength, SideLength,0));
    Mesh.Vertices[2].P = (P + V3(SideLength*2.0f, 0,0));

    return Mesh;
}
#endif

SCENE_HANDLER(HandleSceneFloor)
{
    v3 SourceLight = GameState->DebugSourceLight = V3(0.f,5.0f,0.f);
    {
        entity Entity = {};
        Entity.D              = V3(0,0,1); // RECORD   D;
        Entity.P              = SourceLight; // RECORD   P;
        Entity.dP             = {}; // real32   dP;
        Entity.Mesh           = GameState->Meshes + 0; // mesh * Mesh;
        Entity.Height         = 1.0f; // real32   Height;
        Entity.Scale          = {1,1,1}; // RECORD   Scale;
        RenderMesh(&GameState->Renderer, &Entity, V3(0,0,0),V4(1,1,1,1),V3(0,0,0));
    }

#if 1
    //real32 SinTime = sinf(Input->TimeElapsed);
    real32 SinTime = sinf(Input->TimeElapsed);
    real32 CosTime = cosf(Input->TimeElapsed);
    //SinTime = ((SinTime + 1.0f) * 0.5f) * PI*2.0f;
    SinTime = SinTime;// * PI * 2.0f;
    CosTime = CosTime;// * PI * 2.0f;
    v3 Rotation = V3(SinTime,CosTime,0);
#else
    real32 SinTime = sinf(Input->TimeElapsed);
#endif
    Log("TimeElapsed: %f  Sin: %f Rotation: %f\n",Input->TimeElapsed,SinTime,Rotation.x);
#if 0
    for (uint32 EntityIndex = 0;
                EntityIndex < Scene->EntityCount;
                ++EntityIndex)
    {
        entity * Entity = Scene->Entities[EntityIndex];
        RenderMesh(&GameState->Renderer, Entity, V3(0,0,0));
    }
#else
    v4 Color = V4(0.6f,0.8f,0.2f,1.0f);
    entity * Entity = Scene->Entities[0];
    RenderMesh(&GameState->Renderer, Entity, V3(0,0,0),Color,SourceLight);

    Entity = Scene->Entities[1];
    //Entity->D = V3(SinTime,SinTime,SinTime);
    Color = V4(1.0f,0,0,1.0f);
    RenderMeshAround(&GameState->Renderer,Entity,GameState->Player->P, GameState->Player->D,8.0f,Rotation,Color,SourceLight);

#endif
}

SCENE_LOADER(LoadSceneFloor)
{
    Scene->EntityCount = 0;
    entity * Entity;

#if 1
    Entity = GameState->Entities + GameState->TotalEntities;
    Entity->Mesh = GameState->Meshes + 0;
    Entity->P = V3(0,-2.0f,0);
    Entity->D = V3(0,0,-1);
    Entity->Height = 20.0f;
    Entity->Scale = {8.0f,1.0f,8.0f};
    ++GameState->TotalEntities;
    Scene->Entities[Scene->EntityCount++] = Entity;
#endif

#if 1
    Entity = GameState->Entities + GameState->TotalEntities;
    Entity->Mesh = GameState->Meshes + 0;
    Entity->Height = 20.0f;
    Entity->Scale = {1,1,1};
    ++GameState->TotalEntities;
    Scene->Entities[Scene->EntityCount++] = Entity;
#endif
}

SCENE_LOADER(LoadSceneDebug)
{
    Scene->EntityCount = 0;
    for (uint32 x = 1;
                x < 4;
                ++x)
    {
        entity * Entity = GameState->Entities + GameState->TotalEntities;
        Entity->Mesh = GameState->Meshes + 0;
        real32 y = ((x % 2) ? -1.0f: 1.0f) * 2.0f;
        Entity->P = V3((real32)(x*4),y,-10.0f);
        Entity->Height = 20.0f;
        Entity->D = Normalize(Entity->P - GameState->Player->P);
        ++GameState->TotalEntities;
        Scene->Entities[Scene->EntityCount++] = Entity;
    }
    RenderLookAt(&GameState->Renderer,V3(0,20.0f,5.0f),GameState->Player->P);
}

SCENE_HANDLER(HandleSceneDebug)
{
#if 0
    real32 SinTime = sinf(Input->TimeElapsed);
    SinTime = ((SinTime + 1.0f) * 0.5f) * PI*2.0f;
#else
    real32 SinTime = sinf(Input->TimeElapsed);
#endif
    Log("%f\n",Input->TimeElapsed);
    for (uint32 EntityIndex = 0;
                EntityIndex < (Scene->EntityCount - 1);
                ++EntityIndex)
    {
        entity * Entity = Scene->Entities[EntityIndex];
        //RenderMeshAround(&GameState->Renderer,Entity,GameState->Player->P, GameState->Player->D,3.0f,SinTime);
    }
    entity * Entity = Scene->Entities[Scene->EntityCount - 1];
    Entity->P = V3(0,0,0);
    //RenderMesh(&GameState->Renderer,Entity,V3(0,0,0));
}

void
MoveEntity(entity * Entity, v3 D, real32 dtTime)
{
    real32 Speed = Entity->dP * dtTime;
    Entity->P = Entity->P + (Entity->D * D.z) * Speed;
}

void
FreeCameraView(renderer_3d * Renderer, v3 dP, real32 Yaw, real32 Pitch)
{
    if (dP.z)
    {
        RenderMoveViewForward(Renderer, -dP.z);
    }
    if (dP.x)
    {
        RenderMoveViewRight(Renderer, dP.x);
    }
#if 0
    if (Yaw)
    {
        RenderRotateViewRight(Renderer, ToRadians(Yaw*10.0f));
    }
    if (Pitch)
    {
        RenderRotateViewUp(Renderer, ToRadians(-Pitch*10.0f));
    }
#else
    RenderRotateViewRight(Renderer, ToRadians(Yaw*10.0f));
    RenderRotateViewUp(Renderer, ToRadians(-Pitch*10.0f));
#endif
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

extern "C"
GAME_API
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((Memory->PermanentMemory) && (Memory->PermanentMemorySize > 0))
    Assert((Memory->TransientMemory) && (Memory->TransientMemorySize > 0))
    Assert((sizeof(game_state) <= Memory->PermanentMemorySize));

    game_state * GameState = (game_state *)Memory->PermanentMemory;

    if (!GameState->IsInitialized)
    {
        uint8 * Base = (uint8 *)Memory->TransientMemory;
        InitializeArena(&GameState->TemporaryArena,Base, Memory->TransientMemorySize);

        uint32 ShaderArenaSize = Megabytes(30);
        Base = PushSize(&GameState->TemporaryArena,ShaderArenaSize);
        InitializeArena(&GameState->ShadersArena,Base, ShaderArenaSize);

        uint32 MeshesArenaSize = Megabytes(10);
        Base = PushSize(&GameState->TemporaryArena,MeshesArenaSize);
        InitializeArena(&GameState->MeshesArena, Base, MeshesArenaSize);

        //GameState->Meshes[GameState->TotalMeshes] = LoadModel(Memory,&GameState->MeshesArena,&GameState->TemporaryArena,"assets\\human_male_triangles.obj");
        //++GameState->TotalMeshes;
        GameState->Meshes[GameState->TotalMeshes] = LoadModel(Memory,&GameState->MeshesArena,&GameState->TemporaryArena,"assets\\cube.obj");
        ++GameState->TotalMeshes;

        // TODO: how to properly push vertex inputs to render?
        // Game should tell how much arena?
        // Should buffers be game runtime?
        // or we always reserve a size and then use like we do with cpu memory
        GameState->VertexArena = RenderGetMemoryArena();
        GameState->IndicesArena = RenderGetMemoryArena();

        for (uint32 MeshIndex = 0;
                MeshIndex < GameState->TotalMeshes;
                ++MeshIndex)
        {
            mesh * Mesh = GameState->Meshes + MeshIndex; 

            Mesh->OffsetVertices = GameState->VertexArena.CurrentSize;
            //Mesh->OffsetIndices = GameState->IndicesArena.CurrentSize;

            RenderPushVertexData(&GameState->VertexArena, Mesh->Vertices,Mesh->VertexSize, 1);
            //RenderPushIndexData(&GameState->IndicesArena, Mesh->Indices,Mesh->IndicesSize, 1);
        }

        CreatePipeline(Memory,GameState);

        GameState->Scenes[0].Loader = LoadSceneDebug;
        GameState->Scenes[0].Handler = HandleSceneDebug;
        GameState->Scenes[1].Loader = LoadSceneFloor;
        GameState->Scenes[1].Handler = HandleSceneFloor;

        GameState->CurrentScene = 1;

        v3 WorldCenter = V3(0,0,0);
        GameState->Renderer = 
            CreateRenderer(ToRadians(70.0f), 
                           ScreenWidth,ScreenHeight, 
                           0.1f, 200.0f, 
                           WorldCenter);

        // Load player
        GameState->Player = GameState->Entities + GameState->TotalEntities;
        GameState->Player->P = V3(0.f,3.f,-5.f);
        GameState->Player->Height = 20.0f;
        GameState->Player->Scale = {0.5f, 0.5f, 0.5f};
        GameState->Player->Mesh = GameState->Meshes + 0;
        GameState->Player->D = {0,0,-1};
        GameState->Player->dP = 5.0f;
        ++GameState->TotalEntities;

        GameState->IsInitialized = true;
    }

    if (Input->ShaderHasChanged)
    {
        CreatePipeline(Memory,GameState);
        Input->ShaderHasChanged = false;
    }

    real32 MouseX = (real32)Input->Controller.RelMouseX;
    real32 MouseY = (real32)Input->Controller.RelMouseY;
    
    //if (MouseX != 0.0f || MouseY != 0.0f) Log("x: %f y:%f\n",MouseX,MouseY);

    v3 MouseRotation = V3(MouseX,MouseY,0) / Length(V3(MouseX,MouseY,0));

    scene * Scene = GameState->Scenes + GameState->CurrentScene;

    if (Input->Controller.R.IsPressed)
    {
        if (Scene->Loaded)
        {
            GameState->TotalEntities -= Scene->EntityCount;
            Scene->EntityCount = 0;
        }
        Scene->Loaded = false;
    }

    if (Scene && !Scene->Loaded)
    {
        Scene->Loader(GameState,Scene);
        Scene->Loaded = true;
    }

    /* ------------------------- GAME UPDATE ------------------------- */

    real32 SpeedMetersPerSecond = 7.0f;
    real32 Speed = SpeedMetersPerSecond* (Input->DtFrame);
    v3 dP = {};

    if (Input->Controller.Up.IsPressed)
    {
        dP.z = 1.0f;
    } 
    if (Input->Controller.Down.IsPressed)
    {
        dP.z = -1.0f;
    }
    if (Input->Controller.Left.IsPressed)
    {
        dP.x = -1.0f;
    } 
    if (Input->Controller.Right.IsPressed)
    {
        dP.x = 1.0f;
    }
    if (Input->Controller.Space.IsPressed)
    {
        dP.z = 1.0f;
    }

    real32 Yaw = (MouseX != 0.0f) ? (MouseX / 20.0f) : 0.0f;
    real32 Pitch= (MouseY != 0.0f) ? (MouseY / 20.0f)   : 0.0f;

#if 0
    MoveEntity(GameState->Player, dP,Input->DtFrame);
    //SetViewBehindObject(&GameState->Renderer,GameState->Player->P,GameState->Player->D,5.0f, 2.0f);
#else
    FreeCameraView(&GameState->Renderer, dP, Yaw,Pitch);
#endif

    //RenderLookAt(&GameState->Renderer,V3(0,20.0f,5.0f),GameState->Player->P);
    //v3 RU = RenderGetViewUp(&GameState->Renderer);
    //v3 RU = RenderGetViewDirection(&GameState->Renderer);
    v3 RU = GameState->Player->P;
    //Log("x: %f y: %f z: %f \n", RU.x, RU.y, RU.z);

    /* ------------------------- GAME RENDER ------------------------- */

    //v4 ClearColor = V4(53.0f / 128.0f, 81.0f / 128.0f, 92.0f / 128.0f, 1.0f);
    v4 ClearColor = V4(0,0,0,1);

    BeginRender(GameState, ClearColor);

    v3 CameraP = RenderGetViewPos(&GameState->Renderer);
    v3 CameraD = RenderGetViewDirection(&GameState->Renderer);

    GameState->CameraP = CameraP;
    //GameState->Player->P = CameraP - (CameraD * 3.0f);

    v3 Rotation = {};

    //Rotation.y = -ToRadians(Yaw*10.0f);

    //RenderLookAt(&GameState->Renderer, V3(10.f,10.f,10.0f), GameState->Player->P);
#if 0
    if (Input->Controller.MouseRight.IsPressed && Yaw != 0.0f)
    {
        RenderLookAt(&GameState->Renderer, RenderGetViewPos(&GameState->Renderer), GameState->Player->P + GameState->Player->D);
    }
#endif

    RenderMesh(&GameState->Renderer, GameState->Player, Rotation,V4(1,0,0,1),GameState->DebugSourceLight);

    Scene->Handler(GameState, Input,Scene);

    EndRender(GameState);
}
