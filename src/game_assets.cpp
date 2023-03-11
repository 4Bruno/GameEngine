#include "game_intrinsics.h"
#include "game_assets.h"
#include "game.h"
#include <inttypes.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image_modified.h"


#define ASSETS_MULTITHREAD_ENABLED 0
game_assets
NewGameAssets(memory_arena * Arena, thread_memory_arena * ThreadArenas, i32 LimitThreadArenas)
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
    SubArena(Arena,&Assets.MeshArena,Megabytes(15));

    Assets.ShaderLoadInProgress = 0; // ShaderLoadInProgress   ShaderLoadInProgress
    Assets.MeshLoadInProgress   = 0; // MeshLoadInProgress   MeshLoadInProgress
    Assets.CachedShadersIndex   = ArrayCount(Assets.CachedShaders); // CachedShadersIndex   CachedShadersIndex
    Assets.CachedMeshGroupIndex = ArrayCount(Assets.CachedMeshGroups); // CachedMeshGroupIndex   CachedMeshGroupIndex
    Assets.CachedMaterialIndex  = ArrayCount(Assets.CachedMaterials); // CachedMaterialIndex   CachedMaterialIndex
    Assets.CachedTexturesIndex  = ArrayCount(Assets.CachedTextures); // CachedMaterialIndex   CachedMaterialIndex

    for (u32 ShaderIndex = 0;
            ShaderIndex < ArrayCount(Assets.CachedShaders);
            ++ShaderIndex)
    {
        Assets.CachedShaders[ShaderIndex].GPUID = 0;
        Assets.CachedShaders[ShaderIndex].AssetID = (game_asset_id)0;
    }
    for (u32 MaterialIndex = 0;
            MaterialIndex < ArrayCount(Assets.CachedMaterials);
            ++MaterialIndex)
    {
        Assets.CachedMaterials[MaterialIndex].Pipeline[0] = {};
        Assets.CachedMaterials[MaterialIndex].Pipeline[1] = {};
        Assets.CachedMaterials[MaterialIndex].PipelinesCount = 0;
        Assets.CachedMaterials[MaterialIndex].AssetID = (game_asset_id)0;
    }

    for (u32 TextureIndex = 0;
            TextureIndex < ArrayCount(Assets.CachedTextures);
            ++TextureIndex)
    {
        Assets.CachedTextures[TextureIndex].Width = 0;
        Assets.CachedTextures[TextureIndex].Height = 0;
        Assets.CachedTextures[TextureIndex].Channels = 0;
        Assets.CachedTextures[TextureIndex].AssetID = (game_asset_id)0;
        Assets.CachedTextures[TextureIndex].GPUID = -1;
    }

    for (u32 MeshIndex = 0;
            MeshIndex < ArrayCount(Assets.CachedMeshGroups);
            ++MeshIndex)
    {
        Assets.CachedMeshGroups[MeshIndex].Meshes = PushStruct(&Assets.MeshArena,mesh);
        Assets.CachedMeshGroups[MeshIndex].AssetID = (game_asset_id)0;
        Assets.CachedMeshGroups[MeshIndex].TotalMeshObjects = 0;
        Assets.CachedMeshGroups[MeshIndex].GPUVertexBufferBeginOffset = 0;
        Assets.CachedMeshGroups[MeshIndex].Sphere.c = V3();
        Assets.CachedMeshGroups[MeshIndex].Sphere.r = 0.0f;
    }

    Assets.MeshArenaPermanentSize = 
                    ArrayCount(Assets.CachedMeshGroups) * sizeof(mesh);

    Assets.ThreadArena = ThreadArenas;
    Assets.LimitThreadArenas = LimitThreadArenas;

    // TODO : I don't know where else to put
    stbi_set_flip_vertically_on_load(1);

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
        GetThreadArena(Assets->ThreadArena, Assets->LimitThreadArenas);

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
        "shaders\\default_no_lighting.vert.spv",
        "shaders\\default_lighting.vert.spv",
        "shaders\\fullscreen_triangle.vert.spv",
        "shaders\\triangle.frag.spv",
        "shaders\\triangle_text.frag.spv",
        "shaders\\oit_weighted_color.frag.spv",
        "shaders\\oit_weighted_composite.frag.spv",
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
        case asset_load_inprogress:
        {
            //pass
        } break;
        case asset_loaded_locked:
        {
            // pass;
        } break;
        default:
        {
            Assert(0); // INVALID_PATH_CODE
        };
    };

    if (AssetSlot->State == asset_loaded_on_cpu)
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

    }

    return AssetSlot;
}

#if 1
asset_texture *
GetTexture(game_assets * Assets, game_asset_id ID)
{
    Assert(ID > game_asset_texture_begin && ID < game_asset_texture_end);

    asset_slot * AssetSlot = Assets->AssetSlots + ID;
    asset_state CurrentState = AssetSlot->State;

    u32 LocalArrayIndex = ID - game_asset_texture_begin - 1;

    const char * Paths[ASSETS_TOTAL_TEXTURES] = {
        "assets\\vehicles_001.jpg",
        "assets\\texture_particle_01_small.png"
    };

    struct texture_size
    {
        i32 Width, Height;
    };

    const texture_size TextureSizes[ASSETS_TOTAL_TEXTURES] = {
        {1224, 1632},
        {16,16}
    };
    

    asset_texture * Texture = 0;

    switch (CurrentState)
    {
        case asset_loaded:
        {
            for (i32 CachedTextureIndex = 0;
                    CachedTextureIndex < ArrayCount(Assets->CachedTextures);
                    ++CachedTextureIndex)
            {
                Texture = Assets->CachedTextures + CachedTextureIndex;
                if (Texture->AssetID == ID)
                {
                    break;
                }
            }
            Assert(Texture); // not found?
        } break;
        case asset_unloaded:
        {
            Assert(LocalArrayIndex < ArrayCount(TextureSizes));
            texture_size TextureSize = TextureSizes[LocalArrayIndex];
            u32 TextureSizeBytes = TextureSize.Width * TextureSize.Height * 4;
            u32 ArenaCurrentSize = Assets->MeshArena.CurrentSize;
            u32 ArenaSizeAfterMesh = ArenaCurrentSize + TextureSizeBytes;
            if (ArenaSizeAfterMesh < Assets->MeshArena.MaxSize)
            {
                thread_memory_arena * ThreadArena =
                    GetThreadArena(Assets->ThreadArena, Assets->LimitThreadArenas);

                if (ThreadArena)
                {
                    if (
                            AtomicCompareExchangeU32((volatile u32 *)&Assets->MeshArena.CurrentSize, 
                                ArenaSizeAfterMesh, 
                                ArenaCurrentSize) == ArenaCurrentSize
                       )
                    {
                        u32 Index  = (Assets->CachedTexturesIndex++) & (ArrayCount(Assets->CachedTextures) - 1);

                        asset_texture * CachedTexture = Assets->CachedTextures + Index;
                        CachedTexture->AssetID                 = ID; // ENUM   AssetID

                        if (LoadAssetMultipass(Assets,&Assets->MeshArena, ID, Paths[LocalArrayIndex]))
                        {
                            Assets->MeshLoadInProgress += 1;
                        }

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
            for (i32 CachedTextureIndex = 0;
                    CachedTextureIndex < ArrayCount(Assets->CachedTextures);
                    ++CachedTextureIndex)
            {
                Texture = Assets->CachedTextures + CachedTextureIndex;
                if (Texture->AssetID == ID)
                {
                    break;
                }
            }

            Assert(Texture); // not found?

            u32 MinThreadArenaSize = Megabytes(16);
            thread_memory_arena * ThreadArena =
                    GetThreadArena(Assets->ThreadArena, Assets->LimitThreadArenas, MinThreadArenaSize);


            if (ThreadArena)
            {
                CurrentState = asset_transfer_to_gpu_inprogress;
                void * Data = AssetSlot->Data;
                i32 compressed_size = (u32)AssetSlot->Size;
                texture_size TextureSize = TextureSizes[LocalArrayIndex];
                i32 comp = 0;
                i32 DesiredComp = 4;
                i32 x,y;
                stbi_uc * texture_uncompressed = 
                    stbi_load_from_memory(&ThreadArena->Arena,
                                          (const stbi_uc *)AssetSlot->Data, 
                                          compressed_size, 
                                          &x,&y, &comp, DesiredComp);

                i32 GPUTextureIndex = 
                    GraphicsPushTextureData(
                            texture_uncompressed,
                            TextureSize.Width,TextureSize.Height, 
                            DesiredComp);

                if (GPUTextureIndex >= 0)
                {
                    CurrentState = asset_loaded_on_gpu;
                    Assets->MeshLoadInProgress -= 1;

                    if (Assets->MeshLoadInProgress <= 0)
                    {
                        Assets->MeshArena.CurrentSize = 
                            Assets->MeshArenaPermanentSize;
                    }

                    AssetSlot->State = asset_loaded;
                    Texture->GPUID = GPUTextureIndex;
                    Texture->Width    = x; // Width   Width
                    Texture->Height   = y; // Height   Height
                    Texture->Channels = DesiredComp; // Channels   Channels
                    Texture->AssetID  = ID; // ENUM   AssetID

                } // texture generated

            } // thread arena available

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

    return Texture;

}
#endif

asset_material *
GetMaterial(game_assets * Assets, game_asset_id MaterialID, b32 WaitUntilLoaded)
{
    Assert(MaterialID > game_asset_material_begin && MaterialID < game_asset_material_end);

    asset_material * Material = 0;

#define MATERIAL_INPUTS 4
    const game_asset_id 
          MappingMaterialShaders[ASSETS_TOTAL_MATERIALS][MATERIAL_INPUTS] = 
    {
        {game_asset_shader_vertex_default_no_light, game_asset_shader_fragment_default,           (game_asset_id)0, (game_asset_id)0},
        {game_asset_shader_vertex_default_light,    game_asset_shader_fragment_default,           (game_asset_id)0, (game_asset_id)0},
        {game_asset_shader_vertex_default_light,    game_asset_shader_fragment_default,           (game_asset_id)0, (game_asset_id)0},
        {game_asset_shader_vertex_default_no_light, game_asset_shader_fragment_texture,           (game_asset_id)0, (game_asset_id)0},
        {game_asset_shader_vertex_default_light,    game_asset_shader_fragment_texture,           (game_asset_id)0 , (game_asset_id)0},
        {game_asset_shader_vertex_default_no_light, game_asset_shader_fragment_oit_weighted_color, game_asset_shader_vertex_fullscreen_triangle , game_asset_shader_fragment_oit_weighted_composite }
    };
    const polygon_mode MappingPolygonMode[ASSETS_TOTAL_MATERIALS] =
    {
        polygon_mode_fill,
        polygon_mode_fill,
        polygon_mode_line,
        polygon_mode_fill,
        polygon_mode_fill,
        polygon_mode_fill
    };

    asset_slot * MaterialSlot = Assets->AssetSlots + MaterialID;

    if (MaterialSlot->State == asset_unloaded)
    {
        u32 MaterialLookupIndex = MaterialID - game_asset_material_begin - 1;
        asset_state AssetStateBeforeLock[MATERIAL_INPUTS];
        asset_shader Shaders[MATERIAL_INPUTS];

        b32 AllShadersAreLocked = 0x01;

        for (u32 ShaderIndex = 0; ShaderIndex < MATERIAL_INPUTS; ++ShaderIndex)
        {
            game_asset_id ShaderID = MappingMaterialShaders[MaterialLookupIndex][ShaderIndex];
            if (ShaderID > game_asset_shader_begin && ShaderID < game_asset_shader_end)
            {
                volatile asset_state * AssetState = &GetShader(Assets,ShaderID)->State;
                AssetStateBeforeLock[ShaderIndex] = 
                    (asset_state)AtomicCompareExchangeU32(
                            (volatile u32 *)AssetState, (u32)asset_loaded_locked, (u32)asset_loaded);
                AllShadersAreLocked = AllShadersAreLocked &&
                    (
                        AssetStateBeforeLock[ShaderIndex] == asset_loaded ||
                        AssetStateBeforeLock[ShaderIndex] == asset_loaded_locked
                    );
            }
        }

#if ASSETS_MULTITHREAD_ENABLED
        if (WaitUntilLoaded)
        {
            GlobalPlatformMemory->CompleteWorkQueue(GlobalPlatformMemory->HighPriorityWorkQueue);
            for (u32 ShaderIndex = 0; ShaderIndex < MATERIAL_INPUTS; ++ShaderIndex)
            {
                game_asset_id ShaderID = MappingMaterialShaders[MaterialLookupIndex][ShaderIndex];
                if (ShaderID > game_asset_shader_begin && ShaderID < game_asset_shader_end)
                {
                    volatile asset_state * AssetState = &GetShader(Assets,ShaderID)->State;
                    AssetStateBeforeLock[ShaderIndex] = 
                        (asset_state)AtomicCompareExchangeU32(
                                (volatile u32 *)AssetState, (u32)asset_loaded_locked, (u32)asset_loaded);
                    AllShadersAreLocked = AllShadersAreLocked &&
                        (
                         AssetStateBeforeLock[ShaderIndex] == asset_loaded ||
                         AssetStateBeforeLock[ShaderIndex] == asset_loaded_locked
                        );
                }
            }
            Assert(AllShadersAreLocked);
        }
#endif

        if (AllShadersAreLocked)
        {
            // Create pipeline

            for (u32 ShaderIndex = 0; ShaderIndex < MATERIAL_INPUTS; ++ShaderIndex)
            {
                game_asset_id ShaderID = MappingMaterialShaders[MaterialLookupIndex][ShaderIndex];
                if (ShaderID > game_asset_shader_begin && ShaderID < game_asset_shader_end)
                {
                    Shaders[ShaderIndex] = FindCachedShaderInfo(Assets, ShaderID);
                }
            }

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
                    for (u32 i = 0; i < CachedMaterial->PipelinesCount;++i)
                    {
                        Assert(CachedMaterial->Pipeline[i].Pipeline >= 0);
                        GraphicsDestroyMaterialPipeline(
                                CachedMaterial->Pipeline[i].Pipeline);
                    }
                    // no need, we will set state at the end release lock
                    // AtomicExchangeU32((volatile u32 *)State, (u32)asset_unloaded);
                    break;
                }
                else if (StateBeforeLockAttempt == asset_unloaded)
                {
                    break;
                }
            };

            Assets->CachedMaterialIndex = CachedIndex;

            Assert(CachedMaterial);

            b32 Success = false;
            polygon_mode PolygonMode = MappingPolygonMode[MaterialLookupIndex];

            if (MaterialID == game_asset_material_transparent)
            {
                transparency_pipeline_creation_result TransPipelineResult = 
                    GraphicsCreateTransparencyPipeline(Shaders[0].GPUID,Shaders[1].GPUID,Shaders[2].GPUID,Shaders[3].GPUID);
                CachedMaterial->Pipeline[0] = TransPipelineResult.PipelineCreationResult[0];
                CachedMaterial->Pipeline[1] = TransPipelineResult.PipelineCreationResult[1];
                CachedMaterial->PipelinesCount = 2;
                Success = CachedMaterial->Pipeline[0].Success &&
                          CachedMaterial->Pipeline[1].Success;
            }
            else
            {
                CachedMaterial->Pipeline[0] = 
                    GraphicsCreateMaterialPipeline(Shaders[0].GPUID, Shaders[1].GPUID, PolygonMode);
                CachedMaterial->PipelinesCount = 1;
                Success = CachedMaterial->Pipeline[0].Success;
            }

            if (Success)
            {
                //AtomicExchangeU32((volatile u32 *)&MaterialSlot->State, (u32)asset_loaded);
                MaterialSlot->State = asset_loaded;
                CachedMaterial->AssetID = MaterialID;
                Material = CachedMaterial;
                Logn("Loaded material %i (pipeline %i)", MaterialID, CachedMaterial->Pipeline[0].Pipeline);
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
                Material = Assets->CachedMaterials + CachedIndex;
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

    u32 SizeUniqueUV = Header.TextureCoordCount * sizeof(v2);
    v2 * UniqueTextureArray = (v2 *)PushSize(Arena,SizeUniqueUV);
    Line = 0;

    for (u32 ci = (Header.TextureCoordStart); 
            (Line < Header.TextureCoordCount); 
            ++Line)
    {
        ci += 3; // fixed
        UniqueTextureArray[Line]._V[0] = (r32)atof(Data + ci);
        AdvanceAfterWs(Data,Size,&ci);
        UniqueTextureArray[Line]._V[1] = (r32)atof(Data + ci);
        AdvanceAfterWs(Data,Size,&ci);
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
            VertexBuffer[Indice].UV = UniqueTextureArray[IndexVertexP];
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

        u32 VertexCount = MeshGroup->Meshes[0].VertexSize / sizeof(vertex_point);
        SphereFromDistantPoints(&MeshGroup->Sphere, WorkData->VertexBuffer, VertexCount);

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
GetMesh(game_assets * Assets, game_asset_id ID, b32 WaitUntilLoaded)
{
    Assert(ID > game_asset_mesh_begin && ID < game_asset_mesh_end);

    mesh_group * MeshGroup = 0;

    asset_slot * AssetSlot = Assets->AssetSlots + ID;
    asset_state CurrentState = AssetSlot->State;

    u32 LocalArrayIndex = ID - game_asset_mesh_begin - 1;

    const char * MeshPaths[ASSETS_TOTAL_MESHES] = {
        "assets\\cube_triangles.obj",
        "assets\\quad.obj",
        "assets\\sphere.obj",
        "assets\\tree_001.obj"
    };
    const u32 MeshSizes[ASSETS_TOTAL_MESHES] = {
        36 * sizeof(vertex_point),
        6 * sizeof(vertex_point),
        2880 * sizeof(vertex_point),
        906 * sizeof(vertex_point)
    };
    const u32 MeshObjects[ASSETS_TOTAL_MESHES] = {
        1,
        1,
        1,
        1
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
            b32 HasMemory = ArenaSizeAfterMesh < Assets->MeshArena.MaxSize;
            if (!HasMemory && Assets->MeshLoadInProgress > 1)
            {
                GlobalPlatformMemory->CompleteWorkQueue(GlobalPlatformMemory->HighPriorityWorkQueue);
                ArenaCurrentSize = Assets->MeshArena.CurrentSize;
                ArenaSizeAfterMesh = ArenaCurrentSize + MeshSize;
                HasMemory = ArenaSizeAfterMesh < Assets->MeshArena.MaxSize;
                Assert(HasMemory);
            }
            if (HasMemory)
            {
                thread_memory_arena * ThreadArena =
                    GetThreadArena(Assets->ThreadArena, Assets->LimitThreadArenas);

                if (ThreadArena)
                {
                    // TODO: do I really need atomic op? Who else is touching mesharena?
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
                        if (WaitUntilLoaded)
                        {
                            LoadMesh(0,(void *)AssetLoadData);
                        }
                        else
                        {
                            GlobalPlatformMemory->AddWorkToWorkQueue(
                                    GlobalPlatformMemory->HighPriorityWorkQueue , LoadMesh ,AssetLoadData);
                        }
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
#if 0 // if loading asset synchonously will load to cpu and we will miss this code. Instead IF statement before exit
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
#endif
        case asset_load_inprogress:
        {
            //pass
        } break;
        default:
        {
            Assert(0); // INVALID_PATH_CODE
        };
    };

    if (AssetSlot->State == asset_loaded_on_cpu)
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

    }

    return MeshGroup;
}
