#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include "game_platform.h"
#include "game_memory.h"
#include "game_math.h"
#include "preprocessor_assets.h"
#include "heap.h"


enum asset_state
{
    asset_unloaded        = 0,
    asset_load_inprogress,
    asset_loaded,
    asset_load_error,


    // high word mask
    asset_loaded_on_cpu               = 0b10000000,
    asset_transfer_to_gpu_inprogress  = 0b01000000,
    asset_loaded_on_gpu               = 0b00100000,
    asset_loaded_locked               = 0b00010000
};

struct assets_handler
{
    u32 TagsCount;
    u32 AssetTypeCount;
    u32 AssetsCount;

    bin_tag         * Tags;
    bin_asset_type  * AssetType;
    bin_asset       * Assets;

    // address of the heap block
    uintptr_t   * AssetsMemory;
    asset_state * States;
    i32         * GPUAssetID;

    platform_open_file_result PlatformHandle;

    heap Heap_;
    heap * Heap;
    // to hold asset header info
    memory_arena * Arena;
    // to hold asset data read from the file
    memory_arena * HeapArena;
    thread_memory_arena ThreadArenas[10];

    render_commands_buffer CommandBuffer;
    volatile u32 CommandBufferEntry;
};


struct game_asset
{
    bin_asset   * Asset;
    asset_state * State;
    uintptr_t   * Memory;
};

game_asset
GetMesh(assets_handler * Assets, game_asset_type AssetType ,asset_tag Tag);
game_asset
GetFont(assets_handler * Assets,font_type FontType);
i32
GetShaderVertex(assets_handler * Assets, asset_tag Tag);
i32
GetShaderFragment(assets_handler * Assets, asset_tag Tag);

i32
LoadAsset(assets_handler * Assets, game_asset * Asset, b32 Async);

void
ReleaseAsset(assets_handler * Assets, game_asset * Asset);

inline b32
AssetHasState(game_asset * Asset, asset_state CheckState)
{
    asset_state State = (asset_state)((*Asset->State) & 0x0F);

    return (State == CheckState);
}

inline asset_state
GetAssetStateFlag(game_asset * Asset)
{
    asset_state State = (asset_state)((*Asset->State) & 0xFFF0);

    return State;
}

assets_handler *
InitializeAssets(assets_handler * GameAssets, memory_arena * Arena, memory_arena * HeapArena);

#endif
