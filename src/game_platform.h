#ifndef GAME_PLATFORM_H

#define VK_NO_PROTOTYPES
#include "vulkan\vulkan_core.h"

#define MSVC (_WIN32 || _WIN64)

#if MSVC
#include <intrin.h>
#else
#error OS not supported
#endif

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#define GAME_API __declspec( dllexport )
#define IS_NOT_NULL(Ptr) (Ptr != 0)
#define IS_NULL(Ptr) (Ptr == 0)

#define Kilobytes(x) x*1024
#define Megabytes(x) x*1024*Kilobytes(1)
#define Gigabytes(x) x*1024*Megabytes(1)

#define ArrayCount(a) (sizeof(a) / sizeof(a[0]))
#define global_variable static
#define local_persist   static
#define internal        static

#define BIT_SET(Byte,Bit) ((Byte) & (1<<Bit))

#define Log(format, ...) printf(format, __VA_ARGS__)
#define Logn(format, ...) printf(format "\n", __VA_ARGS__)
#define Assert(exp) if (!(exp)) { Logn("------------- FAILED ASSERTION -------------\nFile:%s\nLine: %i\nExpc:%s",__FILE__,__LINE__,#exp);*(volatile int *)0 = 0; } 

typedef int16_t  i16;
typedef int32_t  i32;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t  b32;
typedef float    r32;
typedef double   r64;

#define PI (r32)3.14159265358979323846


#if DEBUG

// power of 2
#define DEBUG_CYCLE_HISTORY (2 << 5)
struct debug_cycle
{
    u32 RingIndex;
    u32 NumberOfCalls[DEBUG_CYCLE_HISTORY];
    u64 NumberOfCycles[DEBUG_CYCLE_HISTORY];
};


enum debug_cycle_function_name
{
    render_entities   = 0,
    ground_generation = 1,
    begin_simulation  = 2,

    // ALWAYS ABOVE
    debug_cycle_last
};

#define DEBUG_CYCLE_COUNT ((i32)debug_cycle_last)

enum debug_cycle_function
{
    // must be power of 2
    debug_cycle_function_render_entities   = (1 << 0),
    debug_cycle_function_ground_generation = (1 << 1),
    debug_cycle_function_begin_simulation  = (1 << 2)
};

// Toggle on/off function to debug
//#define BITMASK_DEBUG_FUNC debug_cycle_function_render_entities | debug_cycle_function_ground_generation 
#define BITMASK_DEBUG_FUNC debug_cycle_function_render_entities

#define START_CYCLE_COUNT(name) u64 debug_cycle_function##_CycleBegin = __rdtsc();\
                                u32 DebugCycleRingIndex = DebugCycles[name].RingIndex;\
                                DebugCycles[name].NumberOfCalls[DebugCycleRingIndex] += 1;

#define END_CYCLE_COUNT(name) DebugCycles[name].NumberOfCycles[DebugCycleRingIndex] = __rdtsc() - debug_cycle_function##_CycleBegin;

#else

#define START_CYCLE_COUNT(debug_cycle_function)
#define END_CYCLE_COUNT(debug_cycle_function)

#endif

/* ----------------------- BEGIN MULTI THREAD ---------------------- */

// TODO how to get other intrinsics and avoid linking to winin.h??
//#define COMPILER_DONOT_REORDER_BARRIER  __faststorefence()
#define COMPILER_DONOT_REORDER_BARRIER  MemoryBarrier()
struct thread_work_queue;

#define THREAD_DO_WORK_ON_QUEUE(name) b32 name(thread_work_queue * Queue)
typedef THREAD_DO_WORK_ON_QUEUE(thread_do_work_on_queue);

#define THREAD_WORK_HANDLER(name) void name(thread_work_queue * Queue, void * Data)
typedef THREAD_WORK_HANDLER(thread_work_handler);

#define THREAD_ADD_WORK_TO_QUEUE(name) void name(thread_work_queue * Queue, thread_work_handler * Handler, void * Data)
typedef THREAD_ADD_WORK_TO_QUEUE(thread_add_work_to_queue);

#define THREAD_COMPLETE_QUEUE(name) void name(thread_work_queue * Queue)
typedef THREAD_COMPLETE_QUEUE(thread_complete_queue);

struct thread_work_queue_entry
{
    thread_work_handler * Handler;
    void * Data;
};

struct thread_work_queue
{
    void * Semaphore;

    volatile u32 ThingsToDo;
    volatile u32 ThingsDone;

    volatile u32 CurrentRead;
    volatile u32 CurrentWrite;

    thread_work_queue_entry Entries[256];

    thread_do_work_on_queue * DoWorkOnQueue;
};
/* ----------------------- END MULTI THREAD ---------------------- */


struct platform_open_file_result
{
    b32 Success;    
    void * Handle;
    u32 Size;
};

#define PLATFORM_OPEN_FILE_READ_ONLY(name) platform_open_file_result name(const char * Filepath)
typedef PLATFORM_OPEN_FILE_READ_ONLY(platform_open_file_read_only);

#define PLATFORM_OPEN_HANDLE(name) platform_open_file_result name(const char * Filepath)
typedef PLATFORM_OPEN_HANDLE(platform_open_handle);

#define PLATFORM_CLOSE_HANDLE(name) void name(platform_open_file_result OpenFileResult)
typedef PLATFORM_CLOSE_HANDLE(platform_close_handle);

#define PLATFORM_READ_HANDLE(name) b32 name(platform_open_file_result OpenFileResult, void * Buffer, u32 Size, u32 Offset)
typedef PLATFORM_READ_HANDLE(platform_read_handle);

struct platform_api
{
    platform_open_file_read_only  * OpenFileReadOnly;
    platform_open_handle          * OpenHandle;
    platform_close_handle         * CloseHandle;
    platform_read_handle          * ReadHandle;

    thread_complete_queue     * CompleteWorkQueue;
    thread_add_work_to_queue  * AddWorkToWorkQueue;

    thread_work_queue * HighPriorityWorkQueue;
    thread_work_queue * LowPriorityWorkQueue;
    thread_work_queue * RenderWorkQueue;
};

#define DEBUG_OPEN_FILE(name) platform_open_file_result name(const char * Filepath)
typedef DEBUG_OPEN_FILE(debug_open_file);
#define DEBUG_READ_FILE(name) b32 name(platform_open_file_result OpenFileResult, void * Buffer)
typedef DEBUG_READ_FILE(debug_read_file);
#define DEBUG_CLOSE_FILE(name) void name(platform_open_file_result OpenFileResult)
typedef DEBUG_CLOSE_FILE(debug_close_file);

struct render_commands_buffer
{
    u8 * Buffer;
    u32 CurrentSize;
    u32 MaximumSize;

    u32 ElementCount;
};


struct game_button
{
    b32 IsPressed;
    b32 WasPressed;
    u32 Transitions;
};

struct game_controller
{
    r32 MouseX;
    r32 MouseY;
    i32 RelMouseX, RelMouseY;
    game_button Numbers[10];
    union
    {
        game_button Buttons[11];
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
            game_button Lock;
            game_button R;
        };
    };
};
struct game_input
{
    b32 ShaderHasChanged;
    b32 GameDllReloaded;
    b32 CloseGame; // game -> platform
    r32 DtFrame;
    r32 TimeElapsed;
    game_controller Controller;
    b32 LockMouse;
};

struct game_memory
{
    void * PermanentMemory;
    u32 PermanentMemorySize;

    void * TransientMemory;
    u32 TransientMemorySize;

    platform_api PlatformAPI;

#if DEBUG
    debug_cycle * DebugCycle;
#endif

};

struct game_render_commands
{
    u32 MaxBufferSize;
    u32 CurrentBufferSize;
    u8 * BaseAddr;

    u32 ElementCount;
};

struct file_contents
{
    u8 * Base;
    u32 Size;
    b32 Success;
};

#define VULKAN_CREATE_SURFACE(name) i32 name(void * SurfaceData, void * pfnOSSurface, VkInstance Instance, VkSurfaceKHR * Surface)
typedef VULKAN_CREATE_SURFACE(vulkan_create_surface);


#if MSVC
#define CompletePreviousReadsBeforeFutureReads _ReadBarrier()
#define CompletePreviousWritesBeforeFutureWrites _WriteBarrier()
inline b32
CompareAndExchangeIfMatches(volatile u32 * This, u32 IfMatchesThis, u32 UpdateTo)
{
    u32 OriginalValue = 
        _InterlockedCompareExchange(
            (volatile long *)This,
            UpdateTo,
            IfMatchesThis);

    b32 Result = (IfMatchesThis == OriginalValue);

    return Result;
}
#else
#error OS not supported
#endif


// defined in game.h used across all headers
struct game_state;


#define DEBUG_RELEASE_GPU_MEMORY(name) i32 name(i32 AssetID)
typedef DEBUG_RELEASE_GPU_MEMORY(debug_release_gpu_memory);


#define GAME_UPDATE_AND_RENDER(name) void name(game_memory * Memory,game_input * Input, render_commands_buffer * CommandBuffer,  i32 ScreenWidth, i32 ScreenHeight, debug_release_gpu_memory * ReleaseGPUMemory)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#define GAME_PLATFORM_H
#endif
