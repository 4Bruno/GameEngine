#include <Windows.h>
#include "game_platform.h"
#include "win32_io.cpp"
#include "win32_threading.cpp"
#include "vulkan_initializer.cpp"
#include "game_render.h"

#define ENABLED_DEBUG_VULKAN_VALIDATION_LAYER 1

/*extern*/        b32  GlobalAppRunning     = false;
/*extern*/        b32 AllowMouseConfinement = false;
/*extern*/        on_window_resize * GraphicsOnWindowResize = OnWindowResize; // vulkan_initializer.cpp
global_variable   RECT GlobalOldRectClip;
global_variable   RECT GlobalNewRectClip;



struct game_state
{
    HMODULE                  GameLib;
    game_update_and_render * pfnGameUpdateAndRender;
    FILETIME                 CurrentLibLastModified;
    FILETIME                 ShaderVertexLastModified;
};



b32
LoadGameDll(game_state * GameState)
{
    if (GameState->GameLib)
    {
        FreeLibrary(GameState->GameLib);
        GameState->GameLib = 0;
    }


#if DEBUG
    if (!Win32FileExists(GAME_DLL_TEMP))
    {
        return false;
    }

    if (!Win32CopyFile(GAME_DLL_TEMP, GAME_DLL))
    {
        return false;
    }
#endif

    GameState->GameLib = LoadLibrary(GAME_DLL); // HMODULE   Lib;

    if (GameState->GameLib)
    {
        GameState->pfnGameUpdateAndRender = 
            (game_update_and_render *)GetProcAddress(GameState->GameLib, "GameUpdateAndRender"); // game_update_and_render   pfnGameUpdateAndRender;
        GameState->CurrentLibLastModified = Win32GetLastWriteTime((char *)GAME_DLL_TEMP);
        if (!GameState->pfnGameUpdateAndRender)
        {
            FreeLibrary(GameState->GameLib);
            GameState->GameLib = 0;
        }
    }

    b32 Result = (GameState->GameLib && GameState->pfnGameUpdateAndRender);

    return Result;
}

THREAD_DO_WORK_ON_QUEUE(VulkanMemoryTransferHandler)
{
    b32 GoToSleep = false;

    u32 CurrentRead = Queue->CurrentRead;
    u32 NextReadEntry = (CurrentRead + 1) % ArrayCount(Queue->Entries);

    if (CurrentRead != Queue->CurrentWrite)
    {
        if (CompareAndExchangeIfMatches(&Queue->CurrentRead, CurrentRead, NextReadEntry))
        {
            thread_work_queue_entry Entry = Queue->Entries[CurrentRead];

            entry_header      * EntryHeader     = (entry_header *)      Entry.Data;
            entry_push_to_gpu * EntryPushToGPU  = (entry_push_to_gpu *) ((u8 *)Entry.Data + (sizeof(entry_header)));

            bin_asset   * Asset = EntryPushToGPU->Asset;
            asset_state * State = EntryPushToGPU->State;
            void        * Data  = EntryPushToGPU->Data;

            switch (EntryHeader->Type)
            {
                case entry_type_entry_push_texture:
                {
                    bin_text * Texture = &Asset->Text;
                    i32 GPUIndex = PushTextureData(Asset->ID, Data, Texture->Width, Texture->Height, Texture->Channels);
                    Assert(GPUIndex >= 0);
                    *State = asset_loaded;

                } break;
                case entry_type_entry_push_mesh:
                {
                    bin_mesh * Mesh = &Asset->Mesh;
                    void * DataVertices = (u8 *)Data;
                    void * DataIndices  = (u8 *)Data + Mesh->SizeVertices;
                    i32 ErrorCode = PushMeshData(Asset->ID, DataVertices, Mesh->SizeVertices, DataIndices, Mesh->SizeIndices);
                    Assert(ErrorCode == 0);
                    *State = asset_loaded;
                } break;

                case entry_type_entry_push_shader_vertex:
                case entry_type_entry_push_shader_fragment:
                {
                    bin_shader * Shader = &Asset->Shader;
                    Shader->GPUShaderID = CreateShaderModule(Data, EntryPushToGPU->Size);
                    Assert(Shader->GPUShaderID >= 0);
                    *State = asset_loaded;

                } break;
                default:
                {
                    Assert(0); // Vulkan Thread to handle data transfer with incorrect render command type
                };
            };

            InterlockedIncrement((LONG volatile *)EntryPushToGPU->CommandBufferEntry);
            InterlockedIncrement((LONG volatile *)&Queue->ThingsDone);
        }
    }
    else
    {
        GoToSleep = true;
    }


    return GoToSleep;
}


//int main( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
int 
main()
{
    /* ------------------------- THREADS ----------------------------------- */
    thread_work_queue HighPriorityWorkQueue = {};
    u32 HighPriorityWorkQueueThreadCount = 8;
    CreateWorkQueue(&HighPriorityWorkQueue, HighPriorityWorkQueueThreadCount);

    thread_work_queue LowPriorityWorkQueue = {};
    u32 LowPriorityWorkQueueThreadCount = 4;
    CreateWorkQueue(&LowPriorityWorkQueue, LowPriorityWorkQueueThreadCount);

    thread_work_queue RenderWorkQueue = {};
    u32 RenderWorkQueueCount = 1;
    CreateWorkQueue(&RenderWorkQueue, RenderWorkQueueCount, VulkanMemoryTransferHandler);
    /* ------------------------- THREADS ----------------------------------- */

    HINSTANCE hInstance = GetModuleHandle(0);

    HWND WindowHandle = Win32CreateWindow(hInstance);

    /* ------------------------- BEGIN VULKAN ------------------------- */
    vulkan_window_data VulkanWindowData;
    VulkanWindowData.hInstance    = hInstance;    // HINSTANCE   hInstance;
    VulkanWindowData.WindowHandle = WindowHandle; // HWND   WindowHandle;

    graphics_platform_window PlatformWindow;
    PlatformWindow.pfnVulkanCreateSurface      = VulkanCreateSurface; // vulkan_create_surface   * pfnVulkanCreateSurface;
    PlatformWindow.SurfaceData                 = (void *)&VulkanWindowData; // Void * SurfaceData;
    PlatformWindow.VkKHROSSurfaceExtensionName = VK_KHR_WIN32_SURFACE_EXTENSION_NAME; // Char_S * VkKHROSSurfaceExtensionName;
    PlatformWindow.OSSurfaceFuncName           = "vkCreateWin32SurfaceKHR"; // Char_S * OSSurfaceFuncName;

    HMODULE VulkanLib = LoadLibrary("Vulkan-1.dll");
    FARPROC VulkanProcAddr = GetProcAddress(VulkanLib, "vkGetInstanceProcAddr");

    i32 VulkanErrorOnInitialization = 
        InitializeVulkan(APP_WINDOW_WIDTH,APP_WINDOW_HEIGHT, PlatformWindow, ENABLED_DEBUG_VULKAN_VALIDATION_LAYER, (PFN_vkGetInstanceProcAddr)VulkanProcAddr);

    if (VulkanErrorOnInitialization)
    {
        Logn("Unable to load Vulkan");
        return 0;
    }

    GraphicsOnWindowResize = OnWindowResize;

#if DEBUG
    u64 GameCycles[100] = {};
    u32 CurrentCycle = 0;
    debug_cycle DebugCycles[DEBUG_CYCLE_COUNT] = {};
#endif
    /* ------------------------- END VULKAN ------------------------- */


    game_state GameState = {};

    if (LoadGameDll(&GameState))
    {
        /* ------------------------- BEGIN GAME MEMORY ------------------------- */

        u32 PermanentMemorySize = Megabytes(30);
        void * PermanentMemory = Win32AllocateMemory(PermanentMemorySize);
        u32 TransientMemorySize = Gigabytes(1);
        void * TransientMemory = Win32AllocateMemory(TransientMemorySize);

        game_memory GameMemory     = {};

        platform_api PlatformAPI;
        PlatformAPI.OpenHandle            = Win32OpenFile; // platform_open_handle * OpenHandle
        PlatformAPI.OpenFileReadOnly      = Win32OpenFileReadOnly; // platform_open_handle * OpenHandle
        PlatformAPI.CloseHandle           = Win32CloseFile; // platform_close_handle * CloseHandle
        PlatformAPI.ReadHandle            = Win32ReadFile; // platform_read_handle * ReadHandle
                                                           
        PlatformAPI.AddWorkToWorkQueue    = AddWorkToQueue;
        PlatformAPI.CompleteWorkQueue     = CompleteWorkQueue;
        PlatformAPI.HighPriorityWorkQueue = &HighPriorityWorkQueue;
        PlatformAPI.LowPriorityWorkQueue  = &LowPriorityWorkQueue;
        PlatformAPI.RenderWorkQueue       = &RenderWorkQueue;


        GameMemory.PlatformAPI = PlatformAPI;
        GameMemory.PermanentMemory     = PermanentMemory;     // Void * PermanentMemory;
        GameMemory.PermanentMemorySize = PermanentMemorySize; // u32 PermanentMemorySize;
        GameMemory.TransientMemory     = TransientMemory;     // Void * TransientMemory;
        GameMemory.TransientMemorySize = TransientMemorySize; // u32 TransientMemorySize;

        /* ------------------------- END GAME MEMORY ------------------------- */

        game_input Input = {};

        i32 ExpectedFramesPerSecond = 30;
        r32 ExpectedMillisecondsPerFrame = (1.0f / (r32)ExpectedFramesPerSecond) * 1000.0f;
        GlobalAppRunning = true;
        Input.DtFrame = ExpectedMillisecondsPerFrame / 1000.0f;

        win32_frame_timer FrameTimer = NewFrameTimer();

        Win32RegisterRawInput(WindowHandle);
#if DEBUG
        GameMemory.DebugCycle = &DebugCycles[0];
#endif

        // Main loop
        while (GlobalAppRunning)
        {
            BeginFrameTimer(&FrameTimer);

            Input.TimeElapsed = CalculateTimeElapsed(&FrameTimer);

            ResetInput(&Input);

            if (AllowMouseConfinement)
            {
                Win32ConfineCursor(WindowHandle);
            }

            win32_window_dim WinDim = Win32GetWindowSize(WindowHandle);

            Input.Controller.RelMouseX = 0;
            Input.Controller.RelMouseY = 0;

            HandleInput(&Input.Controller,WindowHandle, WinDim.Width, WinDim.Height);

            if (Input.Controller.Lock.IsPressed && !Input.Controller.Lock.WasPressed)
            {
                if (AllowMouseConfinement) Win32ReleaseCursor();
                AllowMouseConfinement = !AllowMouseConfinement;
            }

#if 0
            uint64 GameCycleStart = __rdtsc();
            GameState.pfnGameUpdateAndRender(&GameMemory,&Input,Width, Height);
            GameCycleStart = __rdtsc() - GameCycleStart;

            GameCycles[CurrentCycle++] = GameCycleStart;
            CurrentCycle = CurrentCycle % ArrayCount(GameCycles);

            r64 AvgCycle = 0.0f;
            for (u32 CyCleIndex = 0;
                    CyCleIndex < ArrayCount(GameCycles);
                    ++CyCleIndex)
            {
                AvgCycle += (r64)GameCycles[CyCleIndex];
            }
            AvgCycle /= (r64)ArrayCount(GameCycles);

            //Log("Avg: %f Last: %I64u\n",AvgCycle,GameCycleStart);
#else
            // TODO: having multiple command buffers to pass to GPU
            //       and continue on the CPU side
            render_commands_buffer CommandBuffer = {};
            CommandBuffer.CurrentSize = 0;
            CommandBuffer.MaximumSize = Megabytes(5);
            CommandBuffer.Buffer = (u8 *)Win32AllocateMemory(CommandBuffer.MaximumSize);
            CommandBuffer.ElementCount = 0;

            WaitForRender();

            v4 ClearColor = V4(0,0,0,1);

            RenderBeginPass(ClearColor);
            
            GameState.pfnGameUpdateAndRender(&GameMemory,&Input, &CommandBuffer, WinDim.Width, WinDim.Height, ReleaseGPUMemory);

            u8 * EntryBegin = CommandBuffer.Buffer;
            for (u32 EntryIndex = 0;
                     EntryIndex < CommandBuffer.ElementCount;
                     ++EntryIndex)
            {
                entry_header * Header = (entry_header *)EntryBegin;
                EntryBegin += sizeof(entry_header);

                switch (Header->Type)
                {
                    case entry_type_entry_push_mesh:
                    {
                        entry_push_mesh * EntryMesh = (entry_push_mesh *)EntryBegin;
                        EntryBegin += sizeof(entry_push_mesh);
                        RenderPushMeshIndexedAndDraw(1,EntryMesh->MeshIndex);
                    } break;
                    default:
                    {
                        Logn("GPU render command not implemented");
                        Assert(0);
                    };
                }
            }

            Win32DeallocMemory(CommandBuffer.Buffer);

            EndRenderPass();
#endif
            WaitForFrameTimer(&FrameTimer, ExpectedMillisecondsPerFrame);
#if 0//DEBUG
            const char * DebugCycleFunctionName[] = {
                "render_entities",
                "ground_generation",
                "begin_simulation"
            };

            i32 BitMaskDebugFunc = debug_cycle_function_render_entities;

            for (u32 DebugCycleIndex = 0;
                    DebugCycleIndex < DEBUG_CYCLE_COUNT;
                    ++DebugCycleIndex)
            {
                u32 Index = DebugCycles[DebugCycleIndex].RingIndex;
                u32 FunctCalls = DebugCycles[DebugCycleIndex].NumberOfCalls[Index];
                u64 Cycles = DebugCycles[DebugCycleIndex].NumberOfCycles[Index];
                r32 DebugTimeElapsedMS = QUAD_TO_MS(DebugTimeElapsed);
                u32 Mask = BITMASK_DEBUG_FUNC;
                u32 Check = (1 << DebugCycleIndex);
                if (
                        DebugTimeElapsedMS > 1000.0f && 
                        ((Mask & Check) == Check)
                   )
                {
                    Logn("DEBUG Perf count (%s). Calls: %i Cycles: %I64i",DebugCycleFunctionName[DebugCycleIndex],FunctCalls,Cycles);
                }
                Index = (Index + 1) & (DEBUG_CYCLE_HISTORY - 1);
                DebugCycles[DebugCycleIndex].RingIndex = Index;
                DebugCycles[DebugCycleIndex].NumberOfCalls[Index] = 0;
                DebugCycles[DebugCycleIndex].NumberOfCycles[Index] = 0;
            }

#endif

        } // AppRunning loop

        Win32ReleaseCursor();

        EndFrameTimer();

    } // Game Dll found

    if (VulkanLib)
    {
        CloseVulkan();
        FreeLibrary(VulkanLib);
    }

    if (GameState.GameLib)
    {
        FreeLibrary(GameState.GameLib);
    }
    return 0;

}
