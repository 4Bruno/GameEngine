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

        GameState->TotalMeshes = 1;
        GameState->Meshes[0] = LoadModel(Memory,&GameState->MeshesArena,&GameState->TemporaryArena,"assets\\human_male_triangles.obj");
        //GameState->Meshes[0] = LoadModel(Memory,&GameState->MeshesArena,&GameState->TemporaryArena,"assets\\human_male.obj");
        //GameState->Meshes[0] = LoadModel(Memory,&GameState->MeshesArena,&GameState->TemporaryArena,"assets\\cube.obj");

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

        GameState->VertexArena = RenderGetMemoryArena();
        GameState->IndicesArena = RenderGetMemoryArena();

        GameState->TotalEntities = 2;
        GameState->Entities[0].P = V3(0,0,10);
        GameState->Entities[0].Mesh = GameState->Meshes + 0;

        GameState->Entities[1].P = V3(-5.0f,-5.0f,5.0f);
        GameState->Entities[1].Mesh = GameState->Meshes + 0;


        GameState->CameraP = V3(0,-9.42f, -16.92f);
#if 0

        GameState->TotalMeshes = 3;
        for (uint32 Index = 0;
                    Index < GameState->TotalMeshes;
                    ++Index)
        {
            real32 Offset = (real32)Index*0.2f;
            v3 P = { -1.0f + Offset, 0.0f, 0.0f }; 
            GameState->Meshes[Index] = CreateTriangle(&GameState->MeshesArena,P, 0.2f);
        }
        GameState->TotalMeshes = 1;
        GameState->Meshes[0].Vertices[0].P = V3(-1,-1,0);
        GameState->Meshes[0].Vertices[1].P = V3(0,1,0);
        GameState->Meshes[0].Vertices[2].P = V3(1,-1,0);
#endif

        GameState->IsInitialized = true;
    }


    /* ------------------------- GAME UPDATE ------------------------- */
    real32 Speed = (Input->DtFrame * 0.2f);
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
    // TODO: how to properly push vertex inputs to render?
    // This just works because I am creating the 3 triangles
    // at the same time and the vertices ptr of the 3 triangles
    // happens to be consecutive
    void * BaseVertexAddr = GameState->Meshes[0].Vertices;
    void * BaseIndicesAddr = GameState->Meshes[0].Indices;
    // as for now reset the vertex stack every time
    GameState->VertexArena.CurrentSize = 0;
    GameState->IndicesArena.CurrentSize = 0;

    WaitForRender();

    RenderBeginPass(ClearColor);

    RenderSetPipeline(GameState->PipelineIndex);

    for (uint32 MeshIndex = 0;
                MeshIndex < GameState->TotalMeshes;
                ++MeshIndex)
    {
        mesh * Mesh = GameState->Meshes + MeshIndex; 
        RenderPushVertexData(&GameState->VertexArena, BaseVertexAddr,Mesh->VertexSize, GameState->TotalMeshes);
        RenderPushIndexData(&GameState->IndicesArena, BaseIndicesAddr,Mesh->IndicesSize, GameState->TotalMeshes);
    }

    for (uint32 EntityIndex = 0;
            EntityIndex < GameState->TotalEntities;
            ++EntityIndex)
    {
        entity * Entity = GameState->Entities + EntityIndex;

        m4 Rotation = RotationMatrix(Input->TimeElapsed,V3(0,1,0));
        real32 ScaleFactor = (Input->Controller.Space.IsPressed) ? 2.0f : 1.0f;
        m4 Model = Translate(IdentityMatrix(), (Entity->P - WorldCenter)) * Rotation * ScaleMatrix(ScaleFactor,ScaleFactor,ScaleFactor);
        m4 MeshMatrix = Proj * View * Model;
        MeshMatrix = Transpose(MeshMatrix);

        mesh_push_constant Constants;
        Constants.RenderMatrix = MeshMatrix;

        RenderPushVertexConstant(sizeof(mesh_push_constant),(void *)&Constants);
        RenderPushMesh(GameState->TotalMeshes,GameState->Meshes[0].VertexSize,GameState->Meshes[0].IndicesSize);
    }

    RenderEndPass();
}
