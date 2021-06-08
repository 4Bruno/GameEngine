#include "game_platform.h"
#include "render.h"
#include "math.h"
#include "mesh.h"


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


struct game_state
{
    bool32 IsInitialized;

    memory_arena TemporaryArena;
    memory_arena ShadersArena;
    memory_arena MeshesArena;
    memory_arena VertexArena;

    int32 PipelineIndex;
    int32 VertexShaders[2];
    int32 FragmentShaders[2];

    mesh Meshes[10];
    uint32 TotalMeshes;
};


#define PushSize(Arena,Size) _PushSize(Arena,Size*sizeof(char))
#define PushArray(Arena,Count,Struct) (Struct *)_PushSize(Arena,Count*sizeof(Struct))
#define PushStruct(Arena,Struct) (Struct *)PushArray(Arena,1,Struct)

uint8 *
_PushSize(memory_arena * Arena,uint32 Size)
{
    Assert((Arena->CurrentSize + Size) < Arena->MaxSize);
    uint8 * BaseAddr = Arena->Base + Arena->CurrentSize;
    Arena->CurrentSize += Size;
    return BaseAddr;
}

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

        uint32 MeshesArenaSize = Megabytes(10);
        Base = PushSize(&GameState->TemporaryArena,MeshesArenaSize);
        InitializeArena(&GameState->MeshesArena, Base, MeshesArenaSize);

        GameState->VertexArena = RenderGetMemoryArena();

        GameState->TotalMeshes = 3;
        for (uint32 Index = 0;
                    Index < GameState->TotalMeshes;
                    ++Index)
        {
            real32 Offset = (real32)Index*0.2f;
            v3 P = { -1.0f + Offset, 0.0f, 0.0f }; 
            GameState->Meshes[Index] = CreateTriangle(&GameState->MeshesArena,P, 0.2f);
        }
#if 0
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
    else if (Input->Controller.Down.IsPressed)
    {
        dP.y = -1.0f;
    }
    else if (Input->Controller.Left.IsPressed)
    {
        dP.x = -1.0f;
    } 
    else if (Input->Controller.Right.IsPressed)
    {
        dP.x = 1.0f;
    }
    dP *= Speed;

    for (uint32 Index = 0;
            Index < GameState->TotalMeshes;
            ++Index)
    {
        GameState->Meshes[Index].Vertices[0].P += dP;
        GameState->Meshes[Index].Vertices[1].P += dP;
        GameState->Meshes[Index].Vertices[2].P += dP;
    }

    /* ------------------------- GAME RENDER ------------------------- */
    // TODO: how to properly push vertex inputs to render?
    // This just works because I am creating the 3 triangles
    // at the same time and the vertices ptr of the 3 triangles
    // happens to be consecutive
    void * BaseVertexAddr = GameState->Meshes[0].Vertices;
    // as for now reset the vertex stack every time
    GameState->VertexArena.CurrentSize = 0;
    RenderPushVertexData(&GameState->VertexArena, BaseVertexAddr,GameState->Meshes[0].VertexSize, GameState->TotalMeshes);

    WaitForRender();

    RenderBeginPass(V4(1.0f,0,0,1));

    RenderSetPipeline(GameState->PipelineIndex);

    RenderPushMesh(3,GameState->Meshes[0].VertexSize);

    RenderEndPass();

}
