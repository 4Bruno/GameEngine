#ifndef GAME_PLATFORM_H

#define VK_NO_PROTOTYPES
#include "vulkan\vulkan_core.h"

#include <stdint.h>
#include <stdio.h>

#define GAME_API __declspec( dllexport )

#define Kilobytes(x) x*1024
#define Megabytes(x) x*1024*Kilobytes(1)
#define Gigabytes(x) x*1024*Megabytes(1)

#define Assert(exp) if (!(exp)) { (void)0; } 
#define ArrayCount(a) (sizeof(a) / sizeof(a[0]))
#define global_variable static
#define local_persist   static

#define BIT_SET(Byte,Bit) ((Byte) & (1<<Bit))

#define Log(format, ...) printf(format, __VA_ARGS__)

typedef int32_t     int32;
typedef uint8_t     uint8;
typedef uint32_t    uint32;
typedef uint64_t    uint64;
typedef int32       bool32;
typedef float       real32;


#define VULKAN_CREATE_SURFACE(name) int32 name(void * SurfaceData, void * pfnOSSurface, VkInstance Instance, VkSurfaceKHR * Surface)
typedef VULKAN_CREATE_SURFACE(vulkan_create_surface);

struct platform_open_file_result
{
    bool32 Success;    
    void * Handle;
    uint32 Size;
};

#define DEBUG_OPEN_FILE(name) platform_open_file_result name(const char * Filepath)
typedef DEBUG_OPEN_FILE(debug_open_file);
#define DEBUG_READ_FILE(name) bool32 name(platform_open_file_result OpenFileResult, void * Buffer)
typedef DEBUG_READ_FILE(debug_read_file);
#define DEBUG_CLOSE_FILE(name) void name(platform_open_file_result OpenFileResult)
typedef DEBUG_CLOSE_FILE(debug_close_file);


struct memory_arena
{
    uint8 * Base;
    uint32 MaxSize;
    uint32 CurrentSize;
};

struct game_button
{
    bool32 IsPressed;
    bool32 WasPressed;
    uint32 Transitions;
};

struct game_controller
{
    union
    {
        game_button Buttons[9];
        struct
        {
            game_button Up;
            game_button Down;
            game_button Left;
            game_button Right;
            game_button Space;
            game_button Shift;
            game_button Alt;
            game_button MouseLeft;
            game_button MouseRight;
        };
    };
};
struct game_input
{
    bool32 CloseGame; // game -> platform
    real32 DtFrame;
    real32 TimeElapsed;
    game_controller Controller;
};

struct game_memory
{
    void * PermanentMemory;
    uint32 PermanentMemorySize;

    void * TransientMemory;
    uint32 TransientMemorySize;
    
    debug_open_file  * DebugOpenFile;
    debug_read_file  * DebugReadFile;
    debug_close_file * DebugCloseFile;

};

#define GAME_UPDATE_AND_RENDER(name) void name(game_memory * Memory,game_input * Input)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#define GAME_PLATFORM_H
#endif
