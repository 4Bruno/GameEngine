#include "game_platform.h"
#include "render.h"

struct memory_arena
{
    uint8 * Base;
    uint32 MaxSize;
    uint32 CurrentSize;
};

struct game_state
{
    bool32 IsInitialized;

    memory_arena TemporaryArena;
    memory_arena ShadersArena;

    int32 PipelineIndex;
    VkShaderModule TriangleShaders[2];
};


#define PushSize(Arena,Size) _PushSize(Arena,Size*sizeof(char))
#define PushArray(Arena,Count,Struct) _PushSize(Arena,Count*sizeof(Struct))
#define PushStruct(Arena,Struct) PushArray(Arena,1,Struct)

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

        const char * Shaders[] = {
            "shaders\\triangle.vert",
            "shaders\\triangle.frag"
        };
        for (uint32 ShaderIndex = 0;
                    ShaderIndex < ArrayCount(Shaders);
                    ++ShaderIndex)
        {
            file_contents GetFileResult = GetFileContents(Memory, &GameState->ShadersArena,Shaders[ShaderIndex]);
            if (GetFileResult.Success)
            {
                int32 Result = RenderCreateShaderModule((char *)GetFileResult.Base,
                                                         (size_t)GetFileResult.Size,
                                                         &GameState->TriangleShaders[ShaderIndex]);
                if (Result)
                {
                    Log("Error during creation of shader module %s\n",Shaders[ShaderIndex]);
                }
            }
        }

        GameState->PipelineIndex = 
            RenderCreatePipeline(GameState->TriangleShaders[0], GameState->TriangleShaders[1]);

        if (GameState->PipelineIndex < 0)
        {
            Log("Error during creation of main pipeline\n");
        }

        GameState->IsInitialized = true;
    }

    /* ------------------------- GAME UPDATE ------------------------- */

    /* ------------------------- GAME RENDER ------------------------- */
    WaitForRender();

    RenderLoop(DtTime,GameState->PipelineIndex);
}
