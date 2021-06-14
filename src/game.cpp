#include "game.h"
#include "render.h"
#include "math.h"
#include "model_loader.cpp"



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
        RenderPushMesh(1,Mesh->VertexSize,Mesh->IndicesSize,Mesh->OffsetVertices,Mesh->OffsetIndices);
    }
}

SCENE_LOADER(LoadSceneFloor)
{
    GameState->TotalEntities = 0;

    entity * Entity = GameState->Entities + 0;
    Entity->Mesh = GameState->Meshes + 0;
    Entity->P = V3(0,0,10);
    Entity->Height = 20.0f;
    Entity->Scale = {1,1,1};
    ++GameState->TotalEntities;

#if 0
    Entity = GameState->Entities + 1;
    Entity->Mesh = GameState->Meshes + 1;
    Entity->P = V3(0,0,10);
    Entity->Height = 20.0f;
    //Entity->Scale = {100.0f,100.0f,200.0f};
    Entity->Scale = {1,1,1};
    ++GameState->TotalEntities;
#endif
}

SCENE_LOADER(LoadSceneHumans)
{
    GameState->TotalEntities = 0;

    for (int32 x = -4;
            x < 4;
            ++x)
    {
        for (int32 y = -4;
                y < 4;
                ++y)
        {
            entity * Entity = GameState->Entities + GameState->TotalEntities;
            Entity->Mesh = GameState->Meshes + 0;
            Entity->P = V3((real32)(x*20),(real32)(y*20),10);
            Entity->Height = 20.0f;
            ++GameState->TotalEntities;
        }
    }
}
void
HandleSceneHumans(game_state * GameState,game_input * Input, m4 Proj, m4 View)
{
    v3 WorldCenter = {};
    uint32 RandomNumbers[] = {
        35,92,30,94,71,8,81,28,31,67,62,78,1,7,51,100,29,55,15,50,34,42,23,44,99,45,36,20,12,60,22,72,57,9,98,49,6,95,43,5,53,39,91,3,88,47,2,46,38,76,40,32,18,26,68,93,14,84,27,33,10,69,13,97,63,11,19,48,75,41,80,52,96,87,74,65,56,77,70,61,90,64,24,54,73,79,59,83,37,89,58,66,17,82,16,25,4,21,86,85
    };

    uint32 Seed = 0;
    for (uint32 EntityIndex = 0;
            EntityIndex < GameState->TotalEntities;
            ++EntityIndex)
    {
        entity * Entity = GameState->Entities + EntityIndex;

        if (Seed >= ArrayCount(RandomNumbers)) Seed = 0;
        uint32 Rnd = RandomNumbers[Seed++];
        real32 x = ((Rnd % 2) == 0) ? 1.0f : 0.0f;
        real32 y = ((Rnd % 2) == 1) ? 1.0f : 0.0f;
        m4 Rotation = RotationMatrix(Input->TimeElapsed,V3(x,y,0));
        real32 ScaleFactor = (real32)(Rnd % 4);
        //real32 ScaleFactor = 1.0f;
        m4 Model;
        if (x)
        {
            Model = Translate(IdentityMatrix(), (Entity->P - WorldCenter) + V3(0.5f*Entity->Height,0,0)) * Rotation * ScaleMatrix(ScaleFactor,ScaleFactor,ScaleFactor);
        }
        else
        {
            Model = Translate(IdentityMatrix(), (Entity->P - WorldCenter)) * Rotation * ScaleMatrix(ScaleFactor,ScaleFactor,ScaleFactor);
        }
        m4 MeshMatrix = Proj * View * Model;
        MeshMatrix = Transpose(MeshMatrix);

        mesh_push_constant Constants;
        Constants.RenderMatrix = MeshMatrix;

        RenderPushVertexConstant(sizeof(mesh_push_constant),(void *)&Constants);
        RenderPushMesh(GameState->TotalMeshes,GameState->Meshes[0].VertexSize,GameState->Meshes[0].IndicesSize);
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

        GameState->Meshes[0] = LoadModel(Memory,&GameState->MeshesArena,&GameState->TemporaryArena,"assets\\human_male_triangles.obj");
        ++GameState->TotalMeshes;
        GameState->Meshes[1] = LoadModel(Memory,&GameState->MeshesArena,&GameState->TemporaryArena,"assets\\cube.obj");
        ++GameState->TotalMeshes;

        // TODO: how to properly push vertex inputs to render?
        // This just works because I am creating the 3 triangles
        // at the same time and the vertices ptr of the 3 triangles
        // happens to be consecutive
        void * BaseVertexAddr = GameState->Meshes[0].Vertices;
        void * BaseIndicesAddr = GameState->Meshes[0].Indices;
        // as for now reset the vertex stack every time
        GameState->VertexArena.CurrentSize = 0;
        GameState->IndicesArena.CurrentSize = 0;
        GameState->VertexArena = RenderGetMemoryArena();
        GameState->IndicesArena = RenderGetMemoryArena();

        for (uint32 MeshIndex = 0;
                MeshIndex < GameState->TotalMeshes;
                ++MeshIndex)
        {
            mesh * Mesh = GameState->Meshes + MeshIndex; 

            Mesh->OffsetVertices = GameState->VertexArena.CurrentSize;
            Mesh->OffsetIndices = GameState->IndicesArena.CurrentSize;

            RenderPushVertexData(&GameState->VertexArena, BaseVertexAddr,Mesh->VertexSize, 1);
            RenderPushIndexData(&GameState->IndicesArena, BaseIndicesAddr,Mesh->IndicesSize, 1);
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

        GameState->CurrentScene = 1;

        GameState->CameraP = V3(0,-9.42f, -16.92f);

        GameState->IsInitialized = true;
    }

    scene * Scene = GameState->Scenes + GameState->CurrentScene;
    if (!Scene->Loaded)
    {
        Scene->Loader(GameState);
        Scene->Loaded = true;
    }

    /* ------------------------- GAME UPDATE ------------------------- */
    real32 Speed = (Input->DtFrame);
    v3 dP = {};

    if (Input->Controller.Up.IsPressed)
    {
        dP.y = 1.0f;
    } 
    if (Input->Controller.Down.IsPressed)
    {
        dP.y = -1.0f;
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
        dP.z = -1.0f;
    }
   
    dP *= Speed;

    v3 WorldCenter = V3(0,0,0);
    GameState->CameraP += V3(dP.x,dP.y,dP.z);
    v3 CamPos = GameState->CameraP;

    m4 View = Translate(IdentityMatrix(),CamPos);
    m4 Proj = Projection(ToRadians(70.0f),((real32)ScreenWidth / (real32)ScreenHeight), 0.1f, 200.0f);

    v4 ClearColor = V4(53.0f / 128.0f, 81.0f / 128.0f, 92.0f / 128.0f, 1.0f);

    /* ------------------------- GAME RENDER ------------------------- */

    WaitForRender();

    RenderBeginPass(ClearColor);

    RenderSetPipeline(GameState->PipelineIndex);


    Scene->Handler(GameState, Input,Proj, View);

    RenderEndPass();
}
