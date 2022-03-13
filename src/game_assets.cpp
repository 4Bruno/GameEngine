#include "game_intrinsics.h"
#include "game_assets.h"
#include "game.h"
#include <inttypes.h>

#define ASSETS_MULTITHREAD_ENABLED 0
game_assets
NewGameAssets(memory_arena * Arena)
{

    game_assets Assets;

    Assets.AssetSlots           = PushArray(Arena,ASSETS_TOTAL_COUNT,asset_slot); // Record * AssetSlots
    for (u32 AssetSlotIndex = 0;
            AssetSlotIndex < ASSETS_TOTAL_COUNT;
            ++AssetSlotIndex)
    {
        Assets.AssetSlots[AssetSlotIndex].Size = 0;
        Assets.AssetSlots[AssetSlotIndex].Data = 0;
        Assets.AssetSlots[AssetSlotIndex].State = asset_unloaded;
    }

    SubArena(Arena,&Assets.SoundArena,Megabytes(3));
    SubArena(Arena,&Assets.ShaderArena,Megabytes(3));
    SubArena(Arena,&Assets.MeshArena,Megabytes(3));

    Assets.ShaderLoadInProgress = 0; // ShaderLoadInProgress   ShaderLoadInProgress
    Assets.MeshLoadInProgress   = 0; // MeshLoadInProgress   MeshLoadInProgress
    Assets.CachedShadersIndex   = ArrayCount(Assets.CachedShaders); // CachedShadersIndex   CachedShadersIndex
    Assets.CachedMeshGroupIndex = ArrayCount(Assets.CachedMeshGroups); // CachedMeshGroupIndex   CachedMeshGroupIndex
    Assets.CachedMaterialIndex  = ArrayCount(Assets.CachedMaterials); // CachedMaterialIndex   CachedMaterialIndex

    for (u32 MaterialIndex = 0;
            MaterialIndex < ArrayCount(Assets.CachedMaterials);
            ++MaterialIndex)
    {
        Assets.CachedMaterials[MaterialIndex].Pipeline = {};
        Assets.CachedMaterials[MaterialIndex].AssetID = (game_asset_id)0;
    }

    for (u32 MeshIndex = 0;
            MeshIndex < ArrayCount(Assets.CachedMeshGroups);
            ++MeshIndex)
    {
        Assets.CachedMeshGroups[MeshIndex].Meshes = PushStruct(&Assets.MeshArena,mesh);
    }
    Assets.MeshArenaPermanentSize = 
                    ArrayCount(Assets.CachedMeshGroups) * sizeof(mesh);

    u32 ArenaThreadSize = Megabytes(1);
    u32 TotalThreadsArenaSize = (ArrayCount(Assets.ThreadArena) * ArenaThreadSize);

    for (u32 ThreadArenaIndex = 0;
                ThreadArenaIndex < ArrayCount(Assets.ThreadArena);
                ++ThreadArenaIndex)
    {
        memory_arena * ThreadArena = &Assets.ThreadArena[ThreadArenaIndex].Arena;
        
        ThreadArena->Base = PushSize(Arena, ArenaThreadSize);
        ThreadArena->CurrentSize = 0;
        ThreadArena->MaxSize = ArenaThreadSize;
    }

    return Assets;
}

file_contents
GetFileContents(memory_arena * Arena,const char * Filepath)
{
    file_contents Result = {};
    void * Buffer = 0;

    platform_open_file_result OpenFileResult = GlobalPlatformMemory->PlatformAPI.OpenHandle(Filepath);

    if (OpenFileResult.Success)
    {
        Buffer = PushSize(Arena,OpenFileResult.Size);
        if (GlobalPlatformMemory->PlatformAPI.ReadHandle(OpenFileResult,Buffer))
        {
            Result.Size = OpenFileResult.Size;
            Result.Base = (u8 *)Buffer;
            Result.Success = true;
        }
        else
        {
            Arena->CurrentSize -= OpenFileResult.Size;
        }
        GlobalPlatformMemory->PlatformAPI.CloseHandle(OpenFileResult);
    }
    return Result;
}

struct async_load_asset_data_multipass
{
    game_assets * Assets;
    memory_arena * Arena;
    game_asset_id ID;
    const char * Path;
};

THREAD_WORK_HANDLER(AsyncLoadAssetMultipass)
{
    async_load_asset_data_multipass * AssetLoadData = (async_load_asset_data_multipass *)Data;

    asset_slot * AssetSlot = AssetLoadData->Assets->AssetSlots + AssetLoadData->ID;

    file_contents FileContents = GetFileContents(AssetLoadData->Arena, AssetLoadData->Path);

    if (FileContents.Success)
    {
        AssetSlot->Data = FileContents.Base;
        AssetSlot->Size = FileContents.Size;
        volatile u32 * State = (u32 *)&AssetSlot->State;

        u32 ValueBefore = AtomicExchangeU32(State, asset_loaded_on_cpu);

        Assert(ValueBefore < asset_loaded_on_cpu);
    }
    else
    {
        Assert("Invalid code path. Assets should load, gracefully shutdown?");
    }
}

struct async_load_asset_data
{
    game_assets * Assets;
    thread_memory_arena * ThreadArena;
    game_asset_id ID;
};

THREAD_WORK_HANDLER(AsyncLoadAsset)
{
    async_load_asset_data * AssetLoadData = (async_load_asset_data *)Data;

    asset_slot * AssetSlot = AssetLoadData->Assets->AssetSlots + AssetLoadData->ID;

    const char * FilePath = "";

    file_contents FileContents = GetFileContents(&AssetLoadData->ThreadArena->Arena, FilePath);

    if (FileContents.Success)
    {
        AssetSlot->Data = FileContents.Base;
        AssetSlot->Size = FileContents.Size;

        volatile u32 * State = (u32 *)&AssetSlot->State;

        u32 ValueBefore = AtomicExchangeU32(State, asset_loaded_on_cpu);

        Assert(ValueBefore < asset_loaded_on_cpu);
    }
    else
    {
        Assert("Invalid code path. Assets should load, gracefully shutdown?");
    }

    ThreadEndArena(AssetLoadData->ThreadArena);
}

b32
LoadAsset(game_assets * Assets, game_asset_id ID)
{
    b32 Result = false;

    asset_slot * AssetSlot = Assets->AssetSlots + ID;
    thread_memory_arena * ThreadArena =
        GetThreadArena(&Assets->ThreadArena[0], (u32)ArrayCount(Assets->ThreadArena));

    if (ThreadArena)
    {
        if (
                AtomicCompareExchangeU32((volatile u32 *)&AssetSlot->State, asset_load_inprogress, asset_unloaded)
                ==
                asset_unloaded
           )
        {
            async_load_asset_data * AssetLoadData = PushStruct(&ThreadArena->Arena,async_load_asset_data);
            AssetLoadData->Assets      = Assets;       // Record * Assets
            AssetLoadData->ThreadArena = ThreadArena; // Record * Arena
            AssetLoadData->ID          = ID;           // ENUM ID

#if ASSETS_MULTITHREAD_ENABLED
            GlobalPlatformMemory->AddWorkToWorkQueue(GlobalPlatformMemory->HighPriorityWorkQueue , AsyncLoadAsset ,AssetLoadData);
#else
            AsyncLoadAsset(0,(void *)AssetLoadData);
#endif

            Result = true;
        }
        else
        {
            ThreadEndArena(ThreadArena);
        }
    }

    return Result;
}

b32
LoadAssetMultipass(game_assets * Assets, memory_arena * Arena, game_asset_id ID, const char * Path)
{
    b32 Result = false;

    asset_slot * AssetSlot = Assets->AssetSlots + ID;
#if 0
    if (
            AtomicCompareExchangeU32((volatile u32 *)&AssetSlot->State, asset_load_inprogress, asset_unloaded)
            ==
            asset_unloaded
       )
    {
#endif
    AssetSlot->State = asset_load_inprogress;

    async_load_asset_data_multipass * AssetLoadData = PushStruct(Arena,async_load_asset_data_multipass);
    AssetLoadData->Assets = Assets;       // Record * Assets
    AssetLoadData->Arena  = Arena; // Record * Arena
    AssetLoadData->ID     = ID;           // ENUM ID
    AssetLoadData->Path   = Path;

#if ASSETS_MULTITHREAD_ENABLED
    GlobalPlatformMemory->AddWorkToWorkQueue(GlobalPlatformMemory->HighPriorityWorkQueue , AsyncLoadAssetMultipass ,AssetLoadData);
#else
    AsyncLoadAssetMultipass(0,(void *)AssetLoadData);
#endif

    Result = true;

    return Result;
}

enum game_material
{
    material_default,
    material_textured_with_light,

    material_count
};

asset_shader
FindCachedShaderInfo(game_assets * Assets, game_asset_id ID)
{
    asset_shader Shader = { -1, ID };

    for (u32 CachedIndex = 0; CachedIndex < ArrayCount(Assets->CachedShaders); ++CachedIndex)
    { 
        if (Assets->CachedShaders[CachedIndex].AssetID == ID)
        {
            Shader.GPUID = Assets->CachedShaders[CachedIndex].GPUID;
        }
    }

    return Shader;
}

asset_slot *
GetShader(game_assets * Assets, game_asset_id ID)
{
    Assert(ID > game_asset_shader_begin && ID < game_asset_shader_end);

    asset_slot * AssetSlot = Assets->AssetSlots + ID;
    asset_state CurrentState = AssetSlot->State;

    u32 LocalArrayIndex = ID - game_asset_shader_begin - 1;

    const char * ShaderPaths[game_asset_shader_end - game_asset_shader_begin - 1] = {
        "shaders\\triangle.vert",
        "shaders\\triangle.frag",
        "shaders\\triangle_text.frag"
    };

    switch (CurrentState)
    {
        case asset_loaded:
        {
            // pass
        } break;
        case asset_unloaded:
        {
            if (LoadAssetMultipass(Assets,&Assets->ShaderArena, ID, ShaderPaths[LocalArrayIndex]))
            {
                Assets->ShaderLoadInProgress += 1;
            }
        } break;
        case asset_loaded_on_cpu:
        {
            CurrentState = asset_transfer_to_gpu_inprogress;
            i32 ShaderIndex = GraphicsCreateShaderModule(AssetSlot->Data, AssetSlot->Size);
            Assert(ShaderIndex > -1); // Invalid code path, why failed?

            CurrentState = asset_loaded_on_gpu;
            Assets->ShaderLoadInProgress -= 1;

            if (Assets->ShaderLoadInProgress <= 0)
            {
                Assets->ShaderArena.CurrentSize = 0;
            }

            // ring cache
            u32 BitMask = ArrayCount(Assets->CachedShaders) - 1; // 0x0F 0x1000 - 1 = 0x0111;
            u32 StartIndex = Assets->CachedShadersIndex;
            u32 CachedIndex = ((StartIndex + 1) & BitMask);
            asset_shader * CachedShader = 0;
            for (; 
                 CachedIndex != StartIndex;
                 CachedIndex = (++CachedIndex & BitMask))
            {
                CachedShader = Assets->CachedShaders + CachedIndex;
                volatile asset_state * State = &Assets->AssetSlots[CachedShader->AssetID].State;
                asset_state StateBeforeUpdate = (asset_state)AtomicCompareExchangeU32((volatile u32 *)State, (u32)asset_unloaded, (u32)asset_loaded);
                if (StateBeforeUpdate == asset_loaded)
                {
                    Assert(CachedShader->GPUID > -1);
                    GraphicsDeleteShaderModule(CachedShader->GPUID);
                }
                
                if (StateBeforeUpdate == asset_unloaded || StateBeforeUpdate == asset_loaded)
                {
                    break;
                }
            };

            Assert(CachedShader);

            *CachedShader = { ShaderIndex, ID };

            Assets->CachedShadersIndex = CachedIndex;

            AssetSlot->State = asset_loaded;

        } break;
        default:
        {
            Assert(0); // INVALID_PATH_CODE
        };
    };

    return AssetSlot;
}

asset_material
GetMaterial(game_assets * Assets, game_asset_id MaterialID)
{
    Assert(MaterialID > game_asset_material_begin && MaterialID < game_asset_material_end);

    asset_material Material = {};
    Material.Pipeline.Success = 0;

    // TODO: Dynamic material creation?
    const game_asset_id MappingMaterialShaders[game_asset_material_end - game_asset_material_begin - 1][2] = {
        {game_asset_shader_vertex_default, game_asset_shader_fragment_default},
        {game_asset_shader_vertex_default, game_asset_shader_fragment_texture},
    };

    asset_slot * MaterialSlot = Assets->AssetSlots + MaterialID;

    if (MaterialSlot->State == asset_unloaded)
    {
        u32 MaterialLookupIndex = MaterialID - game_asset_material_begin - 1;

        game_asset_id VertexShaderID = MappingMaterialShaders[MaterialLookupIndex][0];
        game_asset_id FragmentShaderID = MappingMaterialShaders[MaterialLookupIndex][1];

        asset_slot * VertexShaderSlot = GetShader(Assets,VertexShaderID);
        asset_slot * FragmentShaderSlot = GetShader(Assets,FragmentShaderID);
        volatile asset_state * VertexShaderState = &VertexShaderSlot->State;
        volatile asset_state * FragmentShaderState = &FragmentShaderSlot->State;

        asset_state VertexStateAfterLock = (asset_state)AtomicCompareExchangeU32((volatile u32 *)VertexShaderState, (u32)asset_loaded_locked, (u32)asset_loaded);
        asset_state FragmentStateAfterLock = (asset_state)AtomicCompareExchangeU32((volatile u32 *)FragmentShaderState, (u32)asset_loaded_locked, (u32)asset_loaded);

        if (
                (
                 VertexStateAfterLock == asset_loaded ||
                 VertexStateAfterLock == asset_loaded_locked
                ) &&
                (
                 FragmentStateAfterLock == asset_loaded ||
                 FragmentStateAfterLock == asset_loaded_locked
                )
            )
        {
            // Create pipeline
            asset_shader VertexShader = FindCachedShaderInfo(Assets, VertexShaderID);
            asset_shader FragmentShader = FindCachedShaderInfo(Assets, FragmentShaderID);
            
            // ring cache
            u32 BitMask = ArrayCount(Assets->CachedMaterials) - 1; // 0x0F 0x1000 - 1 = 0x0111;
            u32 StartIndex = Assets->CachedMaterialIndex;
            u32 CachedIndex = ((StartIndex + 1) & BitMask);
            asset_material * CachedMaterial = 0;
            for (; 
                 CachedIndex != StartIndex;
                 CachedIndex = (++CachedIndex & BitMask))
            {
                CachedMaterial = Assets->CachedMaterials + CachedIndex;

                // try lock
                volatile asset_state * State = &Assets->AssetSlots[CachedMaterial->AssetID].State;
                asset_state StateBeforeLockAttempt = 
                    (asset_state)AtomicCompareExchangeU32(
                            (volatile u32 *)State, 
                            (u32)asset_loaded_locked, 
                            (u32)asset_loaded);

                if (StateBeforeLockAttempt == asset_loaded)
                {
                    Assert(CachedMaterial->Pipeline.Pipeline >= 0);
                    GraphicsDestroyMaterialPipeline(CachedMaterial->Pipeline.Pipeline);
                    // no need, we will set state at the end release lock
                    // AtomicExchangeU32((volatile u32 *)State, (u32)asset_unloaded);
                    break;
                }
                else if (StateBeforeLockAttempt == asset_unloaded)
                {
                    break;
                }
            };

            Assert(CachedMaterial);

            CachedMaterial->Pipeline = GraphicsCreateMaterialPipeline(VertexShader.GPUID, FragmentShader.GPUID);

            if (CachedMaterial->Pipeline.Success)
            {
                //AtomicExchangeU32((volatile u32 *)&MaterialSlot->State, (u32)asset_loaded);
                MaterialSlot->State = asset_loaded;
                CachedMaterial->AssetID = MaterialID;
                Material = *CachedMaterial;
            }
            else
            {
                MaterialSlot->State = asset_unloaded;
            }
        }
    }
    else if (MaterialSlot->State == asset_loaded)
    {
        for (u32 CachedIndex = 0; CachedIndex < ArrayCount(Assets->CachedMaterials); ++CachedIndex)
        { 
            if (Assets->CachedMaterials[CachedIndex].AssetID == MaterialID)
            {
                Material = Assets->CachedMaterials[CachedIndex];
            }
        }
    }

    return Material;
}


struct async_load_mesh
{
    const char * Path;
    mesh_group  * MeshGroup;
    thread_memory_arena * ThreadArena;
    vertex_point * VertexBuffer;
    asset_slot * AssetSlot;
};

struct parsed_obj_file_result
{
    u32 VertexSize;
    u32 IndicesSize;

    char Name[100];
};

struct obj_file_header
{
    u32  CurrentByteIndex;
    char Name[100];

    u32 VertexStart;    
    u32 VertexCount;

    u32 TextureCoordStart;
    u32 TextureCoordCount;

    u32 VertexNormalStart;
    u32 VertexNormalCount;

    u32 ParameterSpaceVerticesStart;
    u32 ParameterSpaceVerticesCount;

    u32 FaceElementsStart;
    u32 FaceElementsCount;

    u32 LineElementsStart;
    u32 LineElementsCount;
};

inline void
AdvanceAfterWs(const char * Data, u32 Size, u32 * c)
{
    u32 ci = *c;
    for (; (ci < Size);++ci)
    {
        if ( (Data[ci] == ' ') || (Data[ci] == '\n') )
        {
            break;
        }
    }
    *c = ++ci;
}

parsed_obj_file_result
CreateMeshFromObjHeader(obj_file_header Header,
                        memory_arena * Arena,
                        vertex_point * VertexBuffer, 
                        const char * Data, u32 Size,
                        u32 OffsetVertexP, u32 OffsetVertexN)
{
    parsed_obj_file_result Result = {};

    u32 FaceVertices = 3;

    u32 Vertices = Header.FaceElementsCount * FaceVertices; 
    u32 VerticesSize = Vertices * sizeof(vertex_point);

    // Use same arena to temporary allocate
    // unique list of verticesP and normals
    BeginTempArena(Arena,1);

    Result.VertexSize = VerticesSize;

    // create 2 temp arrays for vertices/normals we will shrink stack later
    u32 SizeUniqueVertices = Header.VertexCount * sizeof(v3);
    v3 * UniqueVertexArray = (v3 *)PushSize(Arena,SizeUniqueVertices);

    // Pre-load list of unique vertices
    u32 Line = 0;

    for (u32 ci = (Header.VertexStart); 
            (Line < Header.VertexCount); 
            ++Line)
    {
        ci += 2; // fixed
        for (u32 CoordinateIndex = 0;
                CoordinateIndex < FaceVertices;
                ++CoordinateIndex)
        {
            UniqueVertexArray[Line]._V[CoordinateIndex] = (r32)atof(Data + ci);
            AdvanceAfterWs(Data,Size,&ci);
        }
    }
    
    u32 SizeUniqueNormals = Header.VertexNormalCount * sizeof(v3);
    v3 * UniqueNormalArray = (v3 *)PushSize(Arena,SizeUniqueNormals);

    Line = 0;

    for (u32 ci = (Header.VertexNormalStart); 
            (Line < Header.VertexNormalCount); 
            ++Line)
    {
        ci += 3; // fixed
        for (u32 CoordinateIndex = 0;
                CoordinateIndex < FaceVertices;
                ++CoordinateIndex)
        {
            UniqueNormalArray[Line]._V[CoordinateIndex] = (r32)atof(Data + ci);
            AdvanceAfterWs(Data,Size,&ci);
        }
    }

    // Create all vertices indices data
    char * End;
    Line = 0;
    u32 Indice = 0;
    u32 Base10 = 10;
    for (u32 ci = (Header.FaceElementsStart); 
            (Line < Header.FaceElementsCount); 
            ++Line)
    {
        ci += 2; // fixed
        for (u32 VertexIndex = 0;
                VertexIndex < FaceVertices;
                ++VertexIndex)
        {
            i16 IndexVertexP = (i16)strtoimax(Data + ci,&End, Base10) - (i16)1;
            i16 IndexVertexT = (i16)strtoimax(End + 1,&End, Base10) - (i16)1;
            i16 IndexVertexN = (i16)strtoimax(End + 1,&End, Base10) - (i16)1;

            IndexVertexP = IndexVertexP - (i16)OffsetVertexP;
            IndexVertexN = IndexVertexN - (i16)OffsetVertexN;

            Assert(IndexVertexP <= (i16)Header.VertexCount);
            //Assert(IndexVertexT <= ArrayCount(UniqueVertexArray));
            Assert(IndexVertexN <= (i16)Header.VertexNormalCount);

            // P
            VertexBuffer[Indice].P = UniqueVertexArray[IndexVertexP];
            // TextCoord
            // TODO
            // N
            VertexBuffer[Indice].N = UniqueNormalArray[IndexVertexN];

            AdvanceAfterWs(Data,Size,&ci);
            ++Indice;
        }
    }

    EndTempArena(Arena,1);

    return Result;
}
void
SkipLine(const char * Data, u32 Size,u32 * ci)
{
    u32 c = *ci;
    for (; (c < Size && Data[c] != '\n') ;++c)
    {
    };
    *ci = ++c;
}


inline void
CopyStrUntilChar(char * DestStr,const char * SrcStr, char CharExit, u32 MaxLength)
{
    for (u32 ci = 0;
                ci < MaxLength;
                ++ci)
    {
        if (SrcStr[ci] == CharExit)
        {
            break;
        }
        else
        {
            DestStr[ci] = SrcStr[ci];
        }
    }
}

obj_file_header
ReadObjFileHeader(const char * Data, u32 StartAtByte, u32 Size)
{
    obj_file_header Description = {};

    u32 TotalPolys = 0;
    u32 ci = StartAtByte;


    // find first object
    for (; (ci < Size);)
    {
        if (Data[ci] != 'o') 
        {
            SkipLine(Data,Size,&ci);
        }
        else
        {
            CopyStrUntilChar(Description.Name,Data + ci + 2,'\n',sizeof(Description.Name));
            SkipLine(Data,Size,&ci);
            break;
        }

    }

    for (; (ci < Size);)
    {
        u32 * Count = 0;
        u32 * Start = 0;
        u32 LineCount = 0;
        char ContinueIfA = 0;
        char ContinueIfB = 0;

        // this will tell caller if multiple objects
        // where is the current index
        Description.CurrentByteIndex = ci;

        switch (Data[ci])
        {
            case 'v':
            {
                if ( ((ci + 1) < Size) && (Data[ci+1] == 'p') )
                {
                    Start = &Description.ParameterSpaceVerticesStart;
                    Count = &Description.ParameterSpaceVerticesCount;
                    ContinueIfA = 'v';
                    ContinueIfB = 'p';
                }
                else if ( ((ci + 1) < Size) && (Data[ci+1] == 't') )
                {
                    Start = &Description.TextureCoordStart;
                    Count = &Description.TextureCoordCount;
                    ContinueIfA = 'v';
                    ContinueIfB = 't';
                }
                else if ( ((ci + 1) < Size) && (Data[ci+1] == 'n') )
                {
                    Start = &Description.VertexNormalStart;
                    Count = &Description.VertexNormalCount;
                    ContinueIfA = 'v';
                    ContinueIfB = 'n';
                }
                else
                {
                    Start = &Description.VertexStart;
                    Count = &Description.VertexCount;
                    ContinueIfA = 'v';
                    ContinueIfB = ' ';
                }
            } break;
            case 'f':
            {
                Start = &Description.FaceElementsStart;
                Count = &Description.FaceElementsCount;
                ContinueIfA = 'f';
                ContinueIfB = ' ';
            } break;
            case 'l':
            {
                Start = &Description.LineElementsStart;
                Count = &Description.LineElementsCount;
                ContinueIfA = 'l';
                ContinueIfB = ' ';
            } break;
            case 'm':
            {
                ContinueIfA = 'm';
                ContinueIfB = ' ';
            } break;
            case 'o':
            {
                // Early exit dont parse next object
                // let caller handle it
                return Description;

            } break;
            case 's':
            {
                ContinueIfA = 's';
                ContinueIfB = ' ';
            } break;
            case 'u':
            {
                ContinueIfA = 'u';
                ContinueIfB = ' ';
            } break;
            default:
            {
                // Error incorrect obj file format
                Assert(0);
            } break;
        }

        if (Start)
        {
            *Start = ci;
        }

        SkipLine(Data,Size,&ci);
        ++LineCount;

        for (;((ci + 1) < Size);)
        {
            if ( (Data[ci] == ContinueIfA) && (Data[ci+1] == ContinueIfB) )
            {
                SkipLine(Data,Size,&ci);
                ++LineCount;
            }
            else
            {
                if (Count)
                {
                    *Count = LineCount;
                }
                break;
            }
        }
        // last case exit
        if (Count)
        {
            *Count = LineCount;
        }
    }

#if 0
    Logn("File has %u vertices. Beginning at line %u",Description.VertexCount,Description.VertexStart);
    Logn("File has %u vertices normal. Beginning at line %u",Description.VertexNormalCount,Description.VertexNormalStart);
    Logn("File has %u faces. Beginning at line %u",Description.FaceElementsCount,Description.FaceElementsStart);
#endif


    return Description;
}

THREAD_WORK_HANDLER(LoadMesh)
{
    async_load_mesh * WorkData = (async_load_mesh *)Data;
    Assert(WorkData->ThreadArena);

    mesh_group * MeshGroup = WorkData->MeshGroup;
    Assert(MeshGroup);

    memory_arena * Arena = &WorkData->ThreadArena->Arena;

    i32 Result = -1;
    file_contents GetFileResult = GetFileContents(Arena,WorkData->Path);

    if (GetFileResult.Success)
    {
        u32 CurrentByteIndex = 0;
        // Object file stacks face index position 
        u32 OffsetVertexP = 0;
        u32 OffsetVertexN = 0;
        u32 TotalMeshObjVerticesBytes = 0;

        vertex_point * VertexBuffer = WorkData->VertexBuffer;
        u32 VertexBufferBeginOffset = 0;

        for (u32 MeshObjectIndex = 0;
                MeshObjectIndex < MeshGroup->TotalMeshObjects;
                ++MeshObjectIndex)
        {
            obj_file_header Header = 
                ReadObjFileHeader((const char  *)GetFileResult.Base,CurrentByteIndex, GetFileResult.Size);

            parsed_obj_file_result MeshObj = 
                CreateMeshFromObjHeader(Header, 
                                        Arena,VertexBuffer,
                                        (const char  *)GetFileResult.Base, GetFileResult.Size,
                                        OffsetVertexP,OffsetVertexN);

            mesh * Mesh = MeshGroup->Meshes + MeshObjectIndex;

            Mesh->VertexSize     = MeshObj.VertexSize; // u32   VertexSize;
            Mesh->IndicesSize    = MeshObj.IndicesSize; // u32   IndicesSize;
            VertexBufferBeginOffset = (VertexBufferBeginOffset + MeshObj.VertexSize);
            Mesh->OffsetVertices = VertexBufferBeginOffset;

            VertexBuffer = (VertexBuffer + Mesh->VertexSize);
            CurrentByteIndex = Header.CurrentByteIndex;
            OffsetVertexP += Header.VertexCount;
            OffsetVertexN += Header.VertexNormalCount;
        }

        COMPILER_DONOT_REORDER_BARRIER;
        WorkData->AssetSlot->State = asset_loaded_on_cpu;
    }
    else
    {
        WorkData->AssetSlot->State = asset_load_error;
    }

    ThreadEndArena(WorkData->ThreadArena);

    TempArenaSanityCheck(Arena);
}


mesh_group *
GetMesh(game_assets * Assets, game_asset_id ID)
{
    Assert(ID > game_asset_mesh_begin && ID < game_asset_mesh_end);

    mesh_group * MeshGroup = 0;

    asset_slot * AssetSlot = Assets->AssetSlots + ID;
    asset_state CurrentState = AssetSlot->State;

    u32 LocalArrayIndex = ID - game_asset_mesh_begin - 1;
    const char * MeshPaths[game_asset_mesh_end - game_asset_mesh_begin - 1] = 
    {
        "assets\\cube_triangles.obj"
    };

    const u32 MeshSizes[3] = {
        36 * sizeof(vertex_point)
    };

    switch (CurrentState)
    {
        case asset_loaded:
        {
            for (i32 CachedMeshIndex = 0;
                    CachedMeshIndex < ArrayCount(Assets->CachedMeshGroups);
                    ++CachedMeshIndex)
            {
                MeshGroup = Assets->CachedMeshGroups + CachedMeshIndex;
                if (MeshGroup->AssetID == ID)
                {
                    break;
                }
            }
            Assert(MeshGroup); // not found?
        } break;
        case asset_unloaded:
        {
            u32 MeshSize = MeshSizes[LocalArrayIndex];
            u32 ArenaCurrentSize = Assets->MeshArena.CurrentSize;
            u32 ArenaSizeAfterMesh = ArenaCurrentSize + MeshSize;
            if (ArenaSizeAfterMesh < Assets->MeshArena.MaxSize)
            {
                thread_memory_arena * ThreadArena =
                    GetThreadArena(&Assets->ThreadArena[0], (u32)ArrayCount(Assets->ThreadArena));

                if (ThreadArena)
                {
                    if (
                            AtomicCompareExchangeU32((volatile u32 *)&Assets->MeshArena.CurrentSize, 
                                ArenaSizeAfterMesh, 
                                ArenaCurrentSize) == ArenaCurrentSize
                       )
                    {
                        Assets->MeshLoadInProgress += 1;
                        u32 Index  = (Assets->CachedMeshGroupIndex++) & (ArrayCount(Assets->CachedMeshGroups) - 1);

                        mesh_group * CachedMeshGroup = Assets->CachedMeshGroups + Index;
                        CachedMeshGroup->TotalMeshObjects        = 1; // TotalMeshObjects   TotalMeshObjects
                        CachedMeshGroup->AssetID                 = ID; // ENUM   AssetID
                        void * VertexBuffer = Assets->MeshArena.Base + ArenaCurrentSize;
                        AssetSlot->Data = VertexBuffer;
                        AssetSlot->Size = MeshSize;

                        const char * Path = MeshPaths[LocalArrayIndex];
                        async_load_mesh * AssetLoadData = PushStruct(&ThreadArena->Arena,async_load_mesh);
                        AssetLoadData->Path         = Path;                                                      
                        AssetLoadData->MeshGroup    = CachedMeshGroup;                                           
                        AssetLoadData->ThreadArena  = ThreadArena;                                               
                        AssetLoadData->VertexBuffer = (vertex_point *)VertexBuffer; 
                        AssetLoadData->AssetSlot    = AssetSlot;                                                 

#if ASSETS_MULTITHREAD_ENABLED
                        GlobalPlatformMemory->AddWorkToWorkQueue(
                                GlobalPlatformMemory->HighPriorityWorkQueue , LoadMesh ,AssetLoadData);
#else
                        LoadMesh(0,(void *)AssetLoadData);
#endif

                    }
                    else
                    {
                        ThreadEndArena(ThreadArena);
                    }
                }
            }
        } break;
        case asset_loaded_on_cpu:
        {
            // find placeholder
            for (i32 CachedMeshIndex = 0;
                    CachedMeshIndex < ArrayCount(Assets->CachedMeshGroups);
                    ++CachedMeshIndex)
            {
                MeshGroup = Assets->CachedMeshGroups + CachedMeshIndex;
                if (MeshGroup->AssetID == ID)
                {
                    break;
                }
            }

            Assert(MeshGroup);

            CurrentState = asset_transfer_to_gpu_inprogress;
            void * Data = AssetSlot->Data;
            u32 Size = (u32)AssetSlot->Size;
            i32 ErrorCode = GraphicsPushVertexData(Data, Size, &MeshGroup->GPUVertexBufferBeginOffset);
            if (!ErrorCode)
            {
                CurrentState = asset_loaded_on_gpu;
                Assets->MeshLoadInProgress -= 1;

                if (Assets->MeshLoadInProgress <= 0)
                {
                    Assets->MeshArena.CurrentSize = 
                        Assets->MeshArenaPermanentSize;
                }

                AssetSlot->State = asset_loaded;
            }

        } break;
        case asset_load_inprogress:
        {
            //pass
        } break;
        default:
        {
            Assert(0); // INVALID_PATH_CODE
        };
    };

    return MeshGroup;
}
