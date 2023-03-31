#ifndef GAME_RENDER_H

#define DEFAULT_WORLD_UP V3(0,1,0)
#include "game_math.h"
#include "game_assets.h"
#include "game_platform.h"
//#include "graphics_api.h"
//

struct assets_handler;

enum projection_mode
{
    projection_perspective,
    projection_orthographic
};

enum entry_type
{
    entry_type_unknown,
    entry_type_entry_push_texture,
    entry_type_entry_push_mesh,
    entry_type_entry_push_shader_vertex,
    entry_type_entry_push_shader_fragment
};

struct entry_header
{
    entry_type Type;
};

struct entry_push_mesh
{
    v4 Color;
    m4 Model;
    u32 IndicesSize;
    i32 GPUTextureIndex;
    i32 MeshIndex;
    i32 GPUPipeline;
};

struct entry_push_to_gpu
{
    u32             Size;
    void          * Data;
    bin_asset     * Asset;
    asset_state   * State;

    volatile u32  * CommandBufferEntry;
};


struct renderer
{
    render_commands_buffer * CommandBuffer;
    assets_handler * AssetsManager;
};

void
PushText(renderer * Renderer, const char * Text, font_type Font);
void
InitializeRenderer(renderer * Renderer, assets_handler * Assets, render_commands_buffer * CommandBuffer);


#define GAME_RENDER_H
#endif
