#include "game.h"
#include "vulkan_initializer.h"

platform_api * PlatformAPI = 0;


extern "C"
GAME_API
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((Memory->PermanentMemory) && (Memory->PermanentMemorySize > 0))
    Assert((Memory->TransientMemory) && (Memory->TransientMemorySize > 0))
    Assert((sizeof(game_state) <= Memory->PermanentMemorySize));

    game_state * gs = (game_state *)Memory->PermanentMemory;
    assets_handler * Assets = &gs->AssetsManager; 
    world * World = &gs->World;
    renderer * Renderer = &gs->Renderer;

    if (!gs->IsInitialized)
    {
        PlatformAPI = &Memory->PlatformAPI;

        gs->PermanentMemory = (u8 *)Memory->PermanentMemory;
        InitializeArena(&gs->PermanentArena, gs->PermanentMemory, Memory->PermanentMemorySize);
        PushStruct(&gs->PermanentArena, game_state);

        gs->TransientMemory = (u8 *)Memory->TransientMemory;
        InitializeArena(&gs->TransientArena, gs->TransientMemory, Memory->TransientMemorySize);

        // this is the arena for assets loaded
        memory_arena * AssetsHeapArena = AllocateSubArena(&gs->TransientArena, Megabytes(100));
        // this is the arena for the assets headers and heap struct
        memory_arena * AssetsArena = AllocateSubArena(&gs->PermanentArena, Megabytes(5));
        InitializeAssets(&gs->AssetsManager,AssetsArena, AssetsHeapArena);

        gs->IsInitialized = true;
    }

    InitializeRenderer(Renderer, Assets, CommandBuffer);

    PushText(Renderer, "Hellow world", font_type_times);

#if 0
    for (u32 AssetIndex = 1;
            AssetIndex  < Assets->AssetsCount;
            ++AssetIndex)
    {
        bin_asset * Asset = Assets->Assets + AssetIndex;
        game_asset GameAsset = {};
        GameAsset.Asset = Asset;
        GameAsset.State = Assets->States + AssetIndex;
        GameAsset.Memory = Assets->AssetsMemory + AssetIndex;
        if (AssetHasState(&GameAsset, asset_unloaded))
        {
            LoadAsset(Assets, &GameAsset, false);
        }
    }
    game_asset Quad =
        GetMesh(Assets, game_asset_type_mesh_shape, asset_tag_quad);

    if (AssetHasState(&Quad,asset_unloaded))
    {
        LoadAsset(Assets, &Quad, false);
    }

    game_asset Human =
        GetMesh(Assets, game_asset_type_mesh_humanoid, asset_tag_adult);

    if (AssetHasState(&Human,asset_unloaded))
    {
        LoadAsset(Assets, &Human, false);
    }
    ReleaseAsset(Assets,&Quad);
    ReleaseAsset(Assets,&Human);

#endif

}
