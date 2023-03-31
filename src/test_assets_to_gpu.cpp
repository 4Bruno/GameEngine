#include <Windows.h>
#include "game_platform.h"
#include "game_assets.cpp" 
#include "game_render.cpp"
#include "game_memory.cpp"
#include "hierarchy_tree.cpp"
#include "heap.cpp"
#include "gpu_heap.cpp"
#include "win32_io.cpp"

#define GAME_MULTITHREAD 0
#include "win32_threading.cpp"
#include "vulkan_initializer.cpp"

/*extern*/ platform_api * PlatformAPI;
/*extern*/ b32  GlobalAppRunning     = false;
/*extern*/ b32 AllowMouseConfinement = false;
/*extern*/ on_window_resize * GraphicsOnWindowResize = OnWindowResize;

memory_arena
VirtualAllocArena(u32 Size)
{
    memory_arena Arena = {};
    void * Addr = Win32AllocateMemory(Size);
    InitializeArena(&Arena, Addr, Size);
    return Arena;
}

b32
VulkanMemoryTransferHandler(thread_work_queue * Queue)
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
                    Texture->GPUTextureID = PushTextureData(Data, Texture->Width, Texture->Height, Texture->Channels);
                    Assert(Texture->GPUTextureID >= 0);
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

            InterlockedIncrement((LONG volatile *)&EntryPushToGPU->CommandBufferEntry);
            InterlockedIncrement((LONG volatile *)&Queue->ThingsDone);
        }
    }
    else
    {
        GoToSleep = true;
    }


    return GoToSleep;
}

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
        InitializeVulkan(APP_WINDOW_WIDTH,APP_WINDOW_HEIGHT, PlatformWindow, DEBUG, (PFN_vkGetInstanceProcAddr)VulkanProcAddr);

    if (VulkanErrorOnInitialization)
    {
        Logn("Unable to load Vulkan");
        return 0;
    }

    platform_api PlatformAPI_;
    PlatformAPI_.OpenHandle            = Win32OpenFile; // platform_open_handle * OpenHandle
    PlatformAPI_.OpenFileReadOnly      = Win32OpenFileReadOnly; // platform_open_handle * OpenHandle
    PlatformAPI_.CloseHandle           = Win32CloseFile; // platform_close_handle * CloseHandle
    PlatformAPI_.ReadHandle            = Win32ReadFile; // platform_read_handle * ReadHandle

    PlatformAPI_.AddWorkToWorkQueue    = AddWorkToQueue;
    PlatformAPI_.CompleteWorkQueue     = CompleteWorkQueue;
    PlatformAPI_.HighPriorityWorkQueue = &HighPriorityWorkQueue;
    PlatformAPI_.LowPriorityWorkQueue  = &LowPriorityWorkQueue;
    PlatformAPI_.RenderWorkQueue       = &RenderWorkQueue;

    PlatformAPI = &PlatformAPI_;

    assets_handler Assets_ = {};
    assets_handler * Assets = &Assets_;
   
    memory_arena AssetsArena = VirtualAllocArena(Megabytes(30));
    InitializeAssets(Assets,&AssetsArena, &AssetsArena);

    render_commands_buffer CommandBuffer_ = {};
    render_commands_buffer * CommandBuffer = &CommandBuffer_;
    CommandBuffer->CurrentSize = 0;
    CommandBuffer->MaximumSize = Megabytes(5);
    CommandBuffer->Buffer = (u8 *)Win32AllocateMemory(CommandBuffer->MaximumSize);
    CommandBuffer->ElementCount = 0;

    renderer Renderer_ = {};
    renderer * Renderer = &Renderer_;
    InitializeRenderer(Renderer, Assets, CommandBuffer);

#if 0
    game_asset FontAsset = GetFont(Renderer->AssetsManager,font_type_times);
    game_asset Quad = GetMesh(Renderer->AssetsManager, game_asset_type_mesh_shape, asset_tag_quad);

    if (AssetHasState(&FontAsset, asset_unloaded)) { LoadAsset(Renderer->AssetsManager, &FontAsset, false); }
    if (AssetHasState(&Quad, asset_unloaded))      { LoadAsset(Renderer->AssetsManager, &Quad, false); }
#else
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

        switch (Asset->FileType)
        {
            case asset_file_type_mesh:
                {
                    vertex_point * Vertices = (vertex_point *)(*GameAsset.Memory);

                    u32 CountVertices = Asset->Mesh.SizeVertices / sizeof(vertex_point);
#if 1
                    //for (int i = 0; i < CountVertices;++i)
                    for (u32 vertex_index = 0; vertex_index < min(CountVertices, 5);++vertex_index)
                    {
                        vertex_point * vertex = Vertices + vertex_index;
                        Logn("x:%f y:%f z:%f u:%f v:%f nx:%f ny:%f nz:%f",
                                vertex->P.x,vertex->P.y,vertex->P.z,
                                vertex->UV.x,vertex->UV.y,
                                vertex->N.x,vertex->N.y,vertex->N.z);
                    }
#endif
                } break;

            case asset_file_type_texture:
                {
#if 1
                    bin_text * Text = &Asset->Text;
                    Logn("Texture: width: %i height: %i channels:%i",Text->Height , Text->Width , Text->Channels);
#endif
                } break;

            case asset_file_type_font:
                {
                    bin_text * Text = &Asset->Text;
                    Logn("Font Texture: width: %i height: %i channels:%i",Text->Height , Text->Width , Text->Channels);
                }break;

            case asset_file_type_unknown: break;
            case asset_file_type_sound: break;
            case asset_file_type_shader: 
                {
                    Logn("%s to load shader %i", (Asset->Shader.GPUShaderID >= 0) ? "Success" : "Unsuccessful",AssetIndex);
                } break;
            case asset_file_type_shader_vertex: break;
            case asset_file_type_shader_fragment: break;
            case asset_file_type_shader_geometry: break;
            case asset_file_type_mesh_material: break;
        };

    }

    return 0;
#endif

    game_input Input = {};

    i32 ExpectedFramesPerSecond = 30;
    r32 ExpectedMillisecondsPerFrame = (1.0f / (r32)ExpectedFramesPerSecond) * 1000.0f;
    GlobalAppRunning = true;
    Input.DtFrame = ExpectedMillisecondsPerFrame / 1000.0f;

    win32_frame_timer FrameTimer = NewFrameTimer();

    b32 GameInit = false;
    while (GlobalAppRunning)
    {
        BeginFrameTimer(&FrameTimer);
        Input.TimeElapsed = CalculateTimeElapsed(&FrameTimer);
        ResetInput(&Input);
        win32_window_dim WinDim = Win32GetWindowSize(WindowHandle);
        HandleInput(&Input.Controller,WindowHandle, WinDim.Width, WinDim.Height);

        if (!GameInit)
        {
            GameInit = true;
        }

        WaitForRender();

        v4 ClearColor = V4(0,0,0,1);

        RenderBeginPass(ClearColor);
        
        EndRenderPass();
        //Win32DeallocMemory(CommandBuffer->Buffer);

        WaitForFrameTimer(&FrameTimer, ExpectedMillisecondsPerFrame);
    }

    EndFrameTimer();


}
