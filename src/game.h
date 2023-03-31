#ifndef GAME_H

#include "game_platform.h"
#include "game_math.h"
#include "game_memory.h"
#include "game_assets.h"
#include "heap.h"
#include "game_render.h"

#if DEBUG
extern "C" debug_cycle * DebugCycles;
#endif

extern "C" platform_api * PlatformAPI;

#define RGB_RED   V3(1.0f,0,0)
#define RGB_GREEN V3(0.0f,1.0f,0)
#define RGB_BLUE  V3(0.0f,0.0f,1.0f)
#define RGB_WHITE V3(1.0f,1.0f,1.0f)
#define RGB_GREY  V3(0.5f,0.5f,0.5f)

struct gpu_mesh
{
    u32 OffsetVertices;
    u32 OffsetIndices;
};

struct gpu_text
{
    i32 ID;
    i32 SampleOffsetX;
    i32 SampleOffsetY;
};

struct world 
{
};

struct game_state
{
    u8 * PermanentMemory;
    u8 * TransientMemory;

    memory_arena PermanentArena;
    memory_arena TransientArena;

    thread_memory_arena ThreadArena[16];
    u32 LimitThreadArenas;

    assets_handler AssetsManager;

    world World;

    renderer Renderer;

    b32 IsInitialized;
};

#define GAME_H
#endif
