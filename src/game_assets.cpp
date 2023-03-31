#include "game.h"

assets_handler *
InitializeAssets(assets_handler * GameAssets, memory_arena * Arena, memory_arena * HeapArena)
{
    // TODO: How to handle dev new assets added at runtime?
    platform_open_file_result File = PlatformAPI->OpenFileReadOnly("assets.bin");
    bin_asset_file_header BAF;

    PlatformAPI->ReadHandle(File, &BAF, sizeof(bin_asset_file_header), 0);
    Logn("Total assets %i",BAF.AssetsCount); 

    u32 ByteSizeTags       = BAF.TagCount        * sizeof(bin_tag);
    u32 ByteSizeAssetType  = BAF.AssetsTypeCount * sizeof(bin_asset_type);
    u32 ByteSizeAssets     = BAF.AssetsCount     * sizeof(bin_asset);
    u32 ByteSizeStates     = BAF.AssetsCount     * sizeof(asset_state);
    u32 ByteSizeMemory     = BAF.AssetsCount     * sizeof(uintptr_t);

    GameAssets->TagsCount       = BAF.TagCount;
    GameAssets->AssetTypeCount  = BAF.AssetsTypeCount;
    GameAssets->AssetsCount     = BAF.AssetsCount;

    GameAssets->Tags        = (bin_tag *)       PushSize(Arena, ByteSizeTags);
    GameAssets->AssetType   = (bin_asset_type *)PushSize(Arena, ByteSizeAssetType);
    GameAssets->Assets      = (bin_asset *)     PushSize(Arena, ByteSizeAssets);
    GameAssets->States      = (asset_state *)   PushSize(Arena, ByteSizeStates);
    RtlZeroMemory(GameAssets->States        , ByteSizeStates);
    GameAssets->AssetsMemory= (uintptr_t *)     PushSize(Arena, ByteSizeMemory);
    RtlZeroMemory(GameAssets->AssetsMemory  , ByteSizeMemory);

    PlatformAPI->ReadHandle(File, GameAssets->Tags      , ByteSizeTags      ,BAF.Tags);
    PlatformAPI->ReadHandle(File, GameAssets->AssetType , ByteSizeAssetType ,BAF.AssetTypes);
    PlatformAPI->ReadHandle(File, GameAssets->Assets    , ByteSizeAssets    ,BAF.Assets);

    for (u32 AssetIndex = 1;
            AssetIndex  < GameAssets->AssetsCount;
            ++AssetIndex)
    {
        bin_asset * Asset = GameAssets->Assets + AssetIndex;
        
        Asset->ID = AssetIndex;

        switch (Asset->FileType)
        {
            case asset_file_type_shader:
            {
                Asset->Shader.GPUShaderID     = -1;
            } break;
        }
        Asset->Mesh.GPUIndecesOffset  = UINT64_MAX;
        Asset->Mesh.GPUVerticesOffset = UINT64_MAX;
        Asset->Text.GPUTextureID      = -1;
    }

    GameAssets->Heap = &GameAssets->Heap_;
    InitializeHeap(GameAssets->Heap, HeapArena->Base, HeapArena->MaxSize, 4096);
    GameAssets->HeapArena = HeapArena;

    GameAssets->CommandBuffer       = {};
    u32 NumberCommandBufferEntries  = 50;
    u32 CommandBufferSize           = (sizeof(entry_header) + sizeof(entry_push_to_gpu)) 
                                      * NumberCommandBufferEntries;
    GameAssets->CommandBuffer.MaximumSize = CommandBufferSize;
    GameAssets->CommandBuffer.Buffer      = PushSize(Arena,CommandBufferSize);
    GameAssets->CommandBuffer.CurrentSize = 0;
    GameAssets->CommandBuffer.ElementCount= 0;
    /*
     * Use command buffer as a ring buffer
     *
     */
    GameAssets->CommandBufferEntry = 0;

    GameAssets->Arena           = Arena;
    GameAssets->PlatformHandle  = File;

    return GameAssets;
}

thread_memory_arena *
GetThreadArena(assets_handler * Assets)
{
    thread_memory_arena * Arena = 0;

    for (u32 i = 0;
             i < ArrayCount(Assets->ThreadArenas);
             ++i)
    {
        // Thread safety:
        // State can be changed within a thread to not in use
        // I dont care about false positives
        if (!Assets->ThreadArenas[i].InUse)
        {
            Arena = Assets->ThreadArenas + i;
            Arena->InUse = true;
        }
    }

    return Arena;
}

void
ReleaseAsset(assets_handler * Assets, game_asset * Asset)
{
    if ( CompareAndExchangeIfMatches((volatile u32 *)Asset->State, asset_loaded, asset_unloaded) )
    {
        Assert(Asset->Memory);
        heap_block * Block = (heap_block *)(*Asset->Memory);
        ReleaseBlock(Assets->Heap, Block);
        *Asset->Memory = 0;
        Asset->Memory = 0;
    }
}

void *
GetDataFromHeapMemory(game_asset * Asset)
{
    Assert(Asset->Memory);

    void * Data = (u8 *)(*Asset->Memory) + sizeof(heap_block);

    return Data;
}

i32
LoadAsset(assets_handler * Assets, game_asset * Asset, b32 Async)
{
    if ( CompareAndExchangeIfMatches((volatile u32 *)Asset->State, asset_unloaded, asset_load_inprogress) )
    {
        thread_memory_arena * ThreadArena = 0;

        if (Async)
        {
            ThreadArena = GetThreadArena(Assets);
        }

        if (ThreadArena || !Async)
        {
            u32 TotalSize = Asset->Asset->Size;
            heap_block * MemBlock = FindHeapBlock(Assets->Heap, TotalSize);

            Assert(MemBlock);

            u8 * AssetDataMemory = ((u8 *)Assets->Heap->Memory + MemBlock->Begin);
            *Asset->Memory = (uintptr_t)AssetDataMemory;
            AssetDataMemory += sizeof(heap_block);
            PlatformAPI->ReadHandle(Assets->PlatformHandle, AssetDataMemory, TotalSize, Asset->Asset->DataBeginOffset);

            // POST-PROCESSING
            if (Asset->Asset->FileType <= asset_file_type_requires_gpu_post_processing)
            { 
                // asset on cpu only
                *Asset->State = (asset_state)((*Asset->State) | (asset_state)(asset_loaded_on_cpu));

                render_commands_buffer * CommandBuffer = &Assets->CommandBuffer;
                u32 ReqSize = sizeof(entry_header) + sizeof(entry_push_to_gpu);
                u32 CommandBufferOldSize = CommandBuffer->CurrentSize;
                u32 CommandBufferNewSize = ((CommandBuffer->CurrentSize + ReqSize) % (CommandBuffer->MaximumSize));

                Assert( ( CommandBufferNewSize * (1.0f / ReqSize) ) != Assets->CommandBufferEntry );

                // stall until we get current size
                while (!CompareAndExchangeIfMatches(&CommandBuffer->CurrentSize, CommandBufferOldSize, CommandBufferNewSize))
                {
                    CommandBufferOldSize = CommandBuffer->CurrentSize;
                    CommandBufferNewSize = ((CommandBuffer->CurrentSize + ReqSize) % (CommandBuffer->MaximumSize));
                }

                entry_header      * Header          = (entry_header *)      (CommandBuffer->Buffer  + CommandBufferOldSize);
                entry_push_to_gpu * EntryPushToGPU  = (entry_push_to_gpu *) ((u8 *)Header           + sizeof(entry_header));

                switch (Asset->Asset->AssetType)
                {
                    case game_asset_type_shader_vertex:
                        {
                            Header->Type = entry_type_entry_push_shader_vertex;
                            EntryPushToGPU->Data  = AssetDataMemory; 
                        } break;
                    case game_asset_type_shader_fragment:
                        {
                            Header->Type = entry_type_entry_push_shader_fragment;
                            EntryPushToGPU->Data  = AssetDataMemory; 
                        } break;
                    case game_asset_type_texture_ground:
                        {
                            Header->Type = entry_type_entry_push_texture;
                            EntryPushToGPU->Data  = AssetDataMemory; 
                        } break;
                    case game_asset_type_font:
                        {
                            Header->Type = entry_type_entry_push_texture;
                            AssetDataMemory += (FONT_COUNT * sizeof(font_char_info));
                            EntryPushToGPU->Data  = AssetDataMemory; 
                        } break;
                    case game_asset_type_mesh_humanoid:
                    case game_asset_type_mesh_shape:
                    case game_asset_type_mesh_vegetation:
                        {
                            Header->Type = entry_type_entry_push_mesh;
                            EntryPushToGPU->Data  = AssetDataMemory; 
                        } break;
                    default:
                        {
                            Logn("Not implemented case in asset load to GPU");
                            Assert(0);
                        };
                }
                EntryPushToGPU->Asset = Asset->Asset;
                EntryPushToGPU->Size  = TotalSize;
                EntryPushToGPU->State = Asset->State;

                EntryPushToGPU->CommandBufferEntry = &Assets->CommandBufferEntry;

                PlatformAPI->AddWorkToWorkQueue(PlatformAPI->RenderWorkQueue, 0, (void *)Header);
            } 
            else
            {
                *Asset->State = asset_loaded;
            }

        } // thread or immediate load
    } // unloaded

    return 0;
}

game_asset
GetAsset(assets_handler * Assets, game_asset_type AssetTypeID, bin_tag * MatchVector, r32 * Weights, u32 MatchCount)
{
    game_asset AssetSlot = {};

    bin_asset_type * AssetType = Assets->AssetType + AssetTypeID;

    r32 BestMatch = 10000.0f;
    bin_asset * BestMatchAsset = 0;
    u32 BestAssetIndex  = 0;

    for (u32 AssetIndex = AssetType->Begin;
            AssetIndex  < AssetType->End;
            ++AssetIndex)
    {
        bin_asset * Asset = Assets->Assets + AssetIndex;
        r32 Delta = 0.0f;

        for (u32 VectorIndex = 0;
                 VectorIndex < MatchCount;
                 ++VectorIndex)
        {
            asset_tag ID = MatchVector[VectorIndex].ID;
            r32 Value = MatchVector[VectorIndex].Value;
            r32 Weight = Weights[VectorIndex];

            for (u32 TagIndex = Asset->TagBegin;
                    TagIndex  < Asset->TagOnePastLast;
                    ++TagIndex)
            {
                if (ID == Assets->Tags[TagIndex].ID)
                {
                    Delta += (Assets->Tags[TagIndex].Value - Value) * Weight;
                }
            }
        }
        if (Delta < BestMatch)
        {
            BestMatch = Delta;
            BestMatchAsset = Asset;
            BestAssetIndex = AssetIndex;
        }
    }

    if (BestMatchAsset)
    {
        AssetSlot.Asset = BestMatchAsset;
        AssetSlot.State = Assets->States + BestAssetIndex;
        AssetSlot.Memory = Assets->AssetsMemory + BestAssetIndex;
    }

    return AssetSlot;
}

game_asset
GetFont(assets_handler * Assets,font_type FontType)
{
    bin_tag MatchVector[2] = { 
        {asset_tag_font , (r32)FontType} ,
        {asset_tag_LOD  , 0.0f} 
    };
    r32 Weights[ArrayCount(MatchVector)] = { 
        0.5f , 0.5f
    };

    game_asset Asset = GetAsset(Assets, game_asset_type_font, MatchVector, Weights, ArrayCount(MatchVector));

    return Asset;
}

game_asset
GetMesh(assets_handler * Assets, game_asset_type AssetType ,asset_tag Tag)
{
    bin_tag MatchVector[1] = { Tag, 1.0f };
    r32 Weights[1] = { 1.0f };

    game_asset Asset = GetAsset(Assets, AssetType, MatchVector, Weights, 1);

    return Asset;
}

i32
LoadShader_(assets_handler * Assets, game_asset_type ShaderType, asset_tag Tag)
{
    bin_tag MatchVector[1] = { Tag , 1.0f };
    r32 Weights[1] = { 1.0f };

    game_asset Asset = GetAsset(Assets, ShaderType, MatchVector, Weights, 1);

    i32 VS = -1;

    if (AssetHasState(&Asset,asset_unloaded))
    {
        LoadAsset(Assets, &Asset, false);
    }

    return VS;
}

i32
GetShaderVertex(assets_handler * Assets, asset_tag Tag)
{
    i32 Index = LoadShader_(Assets, game_asset_type_shader_vertex, Tag);

    return Index;
}
i32
GetShaderFragment(assets_handler * Assets, asset_tag Tag)
{
    i32 Index = LoadShader_(Assets, game_asset_type_shader_fragment, Tag);

    return Index;
}


