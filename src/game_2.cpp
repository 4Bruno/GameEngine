#include "game.h"
#include "vulkan_initializer.h"

platform_api * PlatformAPI = 0;
debug_release_gpu_memory * DebugReleaseGPUMemory = 0;

void
LoadAllAssets(assets_handler * Assets)
{
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

        void * AssetMemoryAddr = (void * )*GameAsset.Memory;

        switch (Asset->FileType)
        {
            case asset_file_type_mesh:
                {
                    vertex_point * Vertices = (vertex_point*)AssetMemoryAddr;

                    u32 CountVertices = Asset->Mesh.SizeVertices / sizeof(vertex_point);
                    //for (int i = 0; i < CountVertices;++i)
                    for (u32 vertex_index = 0; vertex_index < min(CountVertices, 5);++vertex_index)
                    {
                        vertex_point * vertex = Vertices + vertex_index;
                        Logn("x:%f y:%f z:%f u:%f v:%f nx:%f ny:%f nz:%f",
                                vertex->P.x,vertex->P.y,vertex->P.z,
                                vertex->UV.x,vertex->UV.y,
                                vertex->N.x,vertex->N.y,vertex->N.z);
                    }
                } break;

            case asset_file_type_texture:
                {
                    bin_text * Text = &Asset->Text;
                    Logn("Texture info: width %i height %i channels %i", Text->Width, Text->Height, Text->Channels);
                } break;

            case asset_file_type_unknown: break;
            case asset_file_type_sound: break;
            case asset_file_type_shader: 
                {
                    Logn("Shader");
                } break;
            case asset_file_type_shader_vertex: break;
            case asset_file_type_shader_fragment: break;
            case asset_file_type_shader_geometry: break;
            case asset_file_type_mesh_material: break;
        };
    }
}

extern "C"
GAME_API
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((Memory->PermanentMemory) && (Memory->PermanentMemorySize > 0))
    Assert((Memory->TransientMemory) && (Memory->TransientMemorySize > 0))
    Assert((sizeof(game_state) <= Memory->PermanentMemorySize));

    game_state     * gs       = (game_state *)Memory->PermanentMemory;
    assets_handler * Assets   = &gs->AssetsManager; 
    world          * World    = &gs->World;
    renderer       * Renderer = &gs->Renderer;

    DebugReleaseGPUMemory = ReleaseGPUMemory;

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
    //PushText(Renderer, "Hellow world", font_type_times);

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
}
