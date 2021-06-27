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
#if 0
    v3 WorldCenter = {};
    for (uint32 EntityIndex = 0;
            EntityIndex < GameState->TotalEntities;
            ++EntityIndex)
    {
        entity * Entity = GameState->Entities + EntityIndex;
        mesh * Mesh = Entity->Mesh;

        m4 Model = Translate(IdentityMatrix(), (Entity->P - WorldCenter)) * ScaleMatrix(Entity->Scale);
        m4 MeshMatrix = Proj * View * Model;
        MeshMatrix = Transpose(MeshMatrix);

        mesh_push_constant Constants;
        Constants.RenderMatrix = MeshMatrix;

        RenderPushVertexConstant(sizeof(mesh_push_constant),(void *)&Constants);
        RenderPushMesh(1,(Mesh->IndicesSize / sizeof(uint16)),Mesh->OffsetVertices,Mesh->OffsetIndices);
    }
#endif
}

SCENE_LOADER(LoadSceneFloor)
{
    entity * Entity;
#if 1
    GameState->TotalEntities = 0;

    Entity = GameState->Entities + 0;
    Entity->Mesh = GameState->Meshes + 0;
    Entity->P = V3(0,0,10);
    Entity->Height = 20.0f;
    Entity->Scale = {1,1,1};
    ++GameState->TotalEntities;
#endif

#if 0
    Entity = GameState->Entities + GameState->TotalEntities;
    Entity->Mesh = GameState->Meshes + 1;
    Entity->P = V3(0,0,10);
    Entity->Height = 20.0f;
    //Entity->Scale = {100.0f,100.0f,200.0f};
    Entity->Scale = {100,100,100};
    ++GameState->TotalEntities;
#endif
}

SCENE_LOADER(LoadSceneHumans)
{
    list * List = &Scene->Entities;
    GameState->TotalEntities = 0;
    for (uint32 x = 1;
                x < 3;
                ++x)
    {
        entity * Entity = GameState->Entities + GameState->TotalEntities;
        Entity->Mesh = GameState->Meshes + 0;
        Entity->P = V3((real32)(x*2),2.0f,-10.0f);
        Entity->Height = 20.0f;
        ++GameState->TotalEntities;
        List->Current = Entity;
        List = List->Next;
    }
}


//HandleSceneHumans(game_state * GameState,game_input * Input, m4 Proj, m4 View)
SCENE_HANDLER(HandleSceneHumans)
{
    real32 SinTime = sinf(Input->TimeElapsed);
    list * List = &Scene->Entities;
    while (List->Current)
    {
        entity * Entity = (entity *)List->Current;
        RenderMeshAround(&GameState->Renderer,Entity,GameState->Player->P,SinTime);
        List = List->Next;
    }
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
            Mesh->OffsetIndices = GameState->IndicesArena.CurrentSize;

            RenderPushVertexData(&GameState->VertexArena, Mesh->Vertices,Mesh->VertexSize, 1);
            RenderPushIndexData(&GameState->IndicesArena, Mesh->Indices,Mesh->IndicesSize, 1);
        }

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


        GameState->Scenes[0].Loader = LoadSceneHumans;
        GameState->Scenes[0].Handler = HandleSceneHumans;
        GameState->Scenes[1].Loader = LoadSceneFloor;
        GameState->Scenes[1].Handler = HandleSceneFloor;

        GameState->CurrentScene = 0;

        v3 WorldCenter = V3(0,0,0);
        GameState->Renderer = 
            CreateRenderer(ToRadians(70.0f), 
                           ScreenWidth,ScreenHeight, 
                           0.1f, 200.0f, 
                           WorldCenter);

        // Load player
        GameState->Player = GameState->Entities + GameState->TotalEntities;
        GameState->Player->P = V3(0.f,3.f,-5.f);
        //GameState->Player->P = V3(0.f,0.f,-5.f);
        GameState->Player->Height = 20.0f;
        GameState->Player->Scale = {0.5f, 0.5f, 0.5f};
        GameState->Player->Mesh = GameState->Meshes + 0;
        GameState->Player->D = {0,0,-1};
        ++GameState->TotalEntities;

        GameState->IsInitialized = true;
    }

    real32 MouseX = (real32)Input->Controller.RelMouseX;
    real32 MouseY = (real32)Input->Controller.RelMouseY;
    
    //if (MouseX != 0.0f || MouseY != 0.0f) Log("x: %f y:%f\n",MouseX,MouseY);

    v3 MouseRotation = V3(MouseX,MouseY,0) / Length(V3(MouseX,MouseY,0));

    scene * Scene = GameState->Scenes + GameState->CurrentScene;

    if (Input->Controller.R.IsPressed)
    {
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
        dP.z = -1.0f;
    } 
    if (Input->Controller.Down.IsPressed)
    {
        dP.z = 1.0f;
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
   
    dP *= Speed;

    v3 WorldCenter = V3(0,0,0);

    real32 Yaw = (MouseX != 0.0f) ? (MouseX / 20.0f) : 0.0f;
    real32 Pitch= (MouseY != 0.0f) ? (MouseY / 20.0f)   : 0.0f;

#if 0
    if (dP.x || dP.y || dP.z)
        RenderMoveView(&GameState->Renderer, V3(dP.x, dP.y, dP.z));
#else
    if (dP.z)
    {
        RenderMoveViewForward(&GameState->Renderer, dP.z);
    }
    if (dP.x)
    {
        RenderMoveViewRight(&GameState->Renderer, dP.x);
    }
#endif

#if 1
    if (Yaw)
    {
        RenderRotateViewRight(&GameState->Renderer, ToRadians(Yaw*10.0f));
    }
    if (Pitch)
    {
        RenderRotateViewUp(&GameState->Renderer, ToRadians(-Pitch*10.0f));
    }
#else
    if (Pitch || Yaw)
        RenderRotateView(&GameState->Renderer, Pitch, -Yaw);

#endif
    //v3 RU = RenderGetViewUp(&GameState->Renderer);
    v3 RU = RenderGetViewDirection(&GameState->Renderer);
    Log("x: %f y: %f z: %f \n", RU.x, RU.y, RU.z);

    /* ------------------------- GAME RENDER ------------------------- */

    v4 ClearColor = V4(53.0f / 128.0f, 81.0f / 128.0f, 92.0f / 128.0f, 1.0f);

    BeginRender(GameState, ClearColor);


    v3 CameraP = RenderGetViewPos(&GameState->Renderer);
    v3 CameraD = RenderGetViewDirection(&GameState->Renderer);

    GameState->Player->P = CameraP - (CameraD * 3.0f);

    v3 Rotation = {};

    Rotation.y = -ToRadians(Yaw*10.0f);
    if (Input->Controller.MouseRight.IsPressed && Yaw != 0.0f)
    {
        RenderLookAt(&GameState->Renderer, RenderGetViewPos(&GameState->Renderer), GameState->Player->P + GameState->Player->D);
    }

    Scene->Handler(GameState, Input,Scene);

    EndRender(GameState);
}
