#include <Windows.h>
#include "game_platform.h"
#include "vulkan\vulkan_win32.h"
#include "render.h"


#define APP_NAME "Vulkan_hardcore"
#define APP_WINDOW_WIDTH  980
#define APP_WINDOW_HEIGHT 540 


#define ENABLED_DEBUG_VULKAN_VALIDATION_LAYER 1

global_variable bool32                  GlobalAppRunning        = false;

struct vulkan_window_data
{
    HINSTANCE hInstance;
    HWND      WindowHandle;
};

VULKAN_CREATE_SURFACE(VulkanCreateSurface)
{
    vulkan_window_data * Window = (vulkan_window_data *)SurfaceData;

    VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo;

    SurfaceCreateInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR; // VkStructureType sType;
    SurfaceCreateInfo.pNext     = 0;                                               // Void * pNext;
    SurfaceCreateInfo.flags     = 0;                                               // VkWin32SurfaceCreateFlagsKHR flags;
    SurfaceCreateInfo.hinstance = Window->hInstance;                               // HINSTANCE hinstance;
    SurfaceCreateInfo.hwnd      = Window->WindowHandle;                            // HWND hwnd;

    PFN_vkCreateWin32SurfaceKHR Fp = (PFN_vkCreateWin32SurfaceKHR)pfnOSSurface;

    if (Fp(Instance,&SurfaceCreateInfo,0, Surface) != VK_SUCCESS)
    {
        Log("Failed to create win32 surface\n");
        return 1;
    }

    return 0;
}

void
UpdateGameButton(game_button * Button, bool32 IsPressed)
{
    Button->WasPressed = !IsPressed;
    Button->IsPressed = IsPressed;
    ++Button->Transitions;
}

void
HandleInput(game_controller * Controller,HWND WindowHandle)
{
    MSG msg = {};
    for (;;)
    { 
        BOOL GotMessage = PeekMessage( &msg, WindowHandle, 0, 0, PM_REMOVE );
        if (!GotMessage) break;
        // https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keydown
        switch (msg.message)
        {
            case WM_QUIT:
            {
                GlobalAppRunning = false;
            } break;

            case WM_SYSKEYDOWN: 
            case WM_SYSKEYUP: 
            case WM_KEYDOWN: 
            case WM_KEYUP:
            {
                uint32 VKCode = (uint32)msg.wParam;

                bool32 IsAltPressed = (msg.lParam & (1 << 29));
                // https://docs.microsoft.com/en-us/windows/win32/inputdev/using-keyboard-input
                bool32 IsShiftPressed = (GetKeyState(VK_SHIFT) & (1 << 15));
                bool32 IsPressed = (msg.lParam & (1UL << 31)) == 0;
                bool32 WasPressed = (msg.lParam & (1 << 30)) != 0;

                if (IsPressed != WasPressed)
                {
                    if (VKCode == VK_ESCAPE) 
                    {
                        GlobalAppRunning = false;
                    }
                    else if (VKCode == VK_UP || VKCode == 'W')
                    {
                        UpdateGameButton(&Controller->Up,IsPressed);
                    }
                    else if (VKCode == VK_DOWN || VKCode == 'S')
                    {
                        UpdateGameButton(&Controller->Down,IsPressed);
                    }
                    else if (VKCode == VK_RIGHT || VKCode == 'D')
                    {
                        UpdateGameButton(&Controller->Right,IsPressed);
                    }
                    else if (VKCode == VK_LEFT || VKCode == 'A')
                    {
                        UpdateGameButton(&Controller->Left,IsPressed);
                    }
                }
            } break;

            default :
            {
                TranslateMessage(&msg); 
                DispatchMessage(&msg); 
            } break;
        }
    } // for lock
}

LRESULT CALLBACK 
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch(message)
    {
        case WM_CLOSE: 
        case WM_QUIT:
        {
            GlobalAppRunning = false;
            PostQuitMessage(0);
        } break;
        case WM_SIZE:
        {
            RECT Rect;
            GetWindowRect(hWnd, &Rect);
            int32 Height =  Rect.bottom - Rect.top;
            int32 Width = Rect.right - Rect.left;
            if (VulkanOnWindowResize(Width,Height))
            {
                Log("Error during swap chain creation\n");
                GlobalAppRunning = false;
            };
        };
        default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        };
    };
    return 0;

}  

HWND
Win32CreateWindow(HINSTANCE hInstance)
{
    WNDCLASS wc      = {0}; 
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = APP_NAME;

    if( !RegisterClass(&wc) )
       return 0;

    HWND WindowHandle = 
        CreateWindow(wc.lpszClassName, 
                    APP_NAME, 
                    WS_OVERLAPPEDWINDOW|WS_VISIBLE, 
                    0,0,APP_WINDOW_WIDTH,APP_WINDOW_HEIGHT,
                    0,0, hInstance,
                    NULL);

    return WindowHandle;
}

struct vulkan_handler
{
    HINSTANCE Lib;
    FARPROC GetInstanceProcAddr;
};

vulkan_handler
GetVulkanHandler()
{
    vulkan_handler VulkanHandler      = {};
    VulkanHandler.Lib                 = LoadLibrary("vulkan-1.dll"); // HINSTANCE   Lib;
    if (VulkanHandler.Lib)
    {
        VulkanHandler.GetInstanceProcAddr = GetProcAddress(VulkanHandler.Lib, "vkGetInstanceProcAddr"); // FARPROC   GetInstanceProcAddr;
    }
    return VulkanHandler;
}

void
CloseFile(platform_open_file_result Result)
{
    if (Result.Handle)
    {
        CloseHandle(Result.Handle);
    }
}

platform_open_file_result
OpenFile(const char * Filepath)
{
    platform_open_file_result Result = {};

    HANDLE FileHandle = CreateFileA(Filepath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);

    if (!FileHandle) return Result;

    LARGE_INTEGER Size;
    if (!GetFileSizeEx(FileHandle, &Size))
    {
        CloseHandle(FileHandle);
        return Result;
    }
    
    Result.Success = true;
    Result.Size    = Size.LowPart;
    Result.Handle  = FileHandle;
       
    return Result;
}

bool32
ReadFile(platform_open_file_result OpenFileResult, void * Buffer)
{

    DWORD BytesRead;

    if (!ReadFile(OpenFileResult.Handle, Buffer, OpenFileResult.Size, &BytesRead, 0) ||
            (OpenFileResult.Size != BytesRead))
    {
        return false;
    }

    return true;

}

struct game_state
{
    HMODULE                Lib;
    game_update_and_render * pfnGameUpdateAndRender;
};

LARGE_INTEGER
Win32QueryPerformance()
{
    LARGE_INTEGER Performance;
    QueryPerformanceCounter(&Performance);
    return Performance;
}
LARGE_INTEGER
Win32QueryPerformanceDiff(LARGE_INTEGER TimeEnd,LARGE_INTEGER TimeStart,LARGE_INTEGER PerformanceFrequency)
{
    TimeEnd.QuadPart = TimeEnd.QuadPart - TimeStart.QuadPart;
    TimeEnd.QuadPart *= 1000000;
    TimeEnd.QuadPart /= PerformanceFrequency.QuadPart;
    return TimeEnd;
}

//int main( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
int main()
{
    HINSTANCE hInstance = GetModuleHandle(0);

    HWND WindowHandle = Win32CreateWindow(hInstance);

    /* ------------------------- BEGIN VULKAN ------------------------- */
    vulkan_window_data VulkanWindowData;
    VulkanWindowData.hInstance    = hInstance;    // HINSTANCE   hInstance;
    VulkanWindowData.WindowHandle = WindowHandle; // HWND   WindowHandle;

    vulkan_handler Vulkan = GetVulkanHandler();

    if (!Vulkan.Lib || !Vulkan.GetInstanceProcAddr)
    {
        Log("Couldn't load vulkan library/instance proc address\n");
        return 1;
    }

    vulkan_platform_window PlatformWindow;
    PlatformWindow.pfnVulkanCreateSurface      = VulkanCreateSurface; // vulkan_create_surface   * pfnVulkanCreateSurface;
    PlatformWindow.SurfaceData                 = (void *)&VulkanWindowData; // Void * SurfaceData;
    PlatformWindow.VkKHROSSurfaceExtensionName = VK_KHR_WIN32_SURFACE_EXTENSION_NAME; // Char_S * VkKHROSSurfaceExtensionName;
    PlatformWindow.OSSurfaceFuncName           = "vkCreateWin32SurfaceKHR"; // Char_S * OSSurfaceFuncName;

    if (InitializeVulkan(APP_WINDOW_WIDTH,APP_WINDOW_HEIGHT,
                         PlatformWindow,
                         ENABLED_DEBUG_VULKAN_VALIDATION_LAYER,
                         (PFN_vkGetInstanceProcAddr)Vulkan.GetInstanceProcAddr))
    {
        Log("Failed to initialize Vulkan\n");
        return 1;
    }
    /* ------------------------- END VULKAN ------------------------- */


    /* ------------------------- BEGIN GAME MEMORY ------------------------- */
    game_state GameState = {};

    GameState.Lib = LoadLibrary("game.dll"); // HMODULE   Lib;

    if (GameState.Lib)
    {
        GameState.pfnGameUpdateAndRender = 
            (game_update_and_render *)GetProcAddress(GameState.Lib, "GameUpdateAndRender"); // game_update_and_render   pfnGameUpdateAndRender;
    }

    if (GameState.pfnGameUpdateAndRender)
    {
        uint32 PermanentMemorySize = Megabytes(30);
        void * PermanentMemory = VirtualAlloc(0, PermanentMemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        uint32 TransientMemorySize = Gigabytes(1);
        void * TransientMemory = VirtualAlloc(0, TransientMemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        game_memory GameMemory     = {};

        GameMemory.DebugOpenFile       = OpenFile;            // debug_open_file DebugOpenFile;
        GameMemory.DebugReadFile       = ReadFile;            // debug_read_file DebugReadFile;
        GameMemory.DebugCloseFile      = CloseFile;           // debug_close_file DebugCloseFile;
        GameMemory.PermanentMemory     = PermanentMemory;     // Void * PermanentMemory;
        GameMemory.PermanentMemorySize = PermanentMemorySize; // uint32 PermanentMemorySize;
        GameMemory.TransientMemory     = TransientMemory;     // Void * TransientMemory;
        GameMemory.TransientMemorySize = TransientMemorySize; // uint32 TransientMemorySize;
        /* ------------------------- END GAME MEMORY ------------------------- */

        int32 ExpectedFramesPerSecond = 30;
        real32 ExpectedMillisecondsPerFrame = 1.0f / (real32)ExpectedFramesPerSecond;
        GlobalAppRunning = true;
        game_input Input = {};
        Input.DtFrame = ExpectedMillisecondsPerFrame;

        //https://docs.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stampsp
        LARGE_INTEGER PerfFreq;
        LARGE_INTEGER StartingTime;
        QueryPerformanceFrequency(&PerfFreq);
        QueryPerformanceCounter(&StartingTime);

        // Main loop
        while (GlobalAppRunning)
        {
            LARGE_INTEGER TimeElapsed = 
                Win32QueryPerformanceDiff(Win32QueryPerformance(), StartingTime, PerfFreq);
            Input.TimeElapsed = (real32)(TimeElapsed.QuadPart * (1.0f / 1000000.0f));

            HandleInput(&Input.Controller,WindowHandle);

            GameState.pfnGameUpdateAndRender(&GameMemory,&Input);

            //Log("Time elapsed %f\n",(real32)(TimeElapsed.QuadPart * (1.0f / 1000000.0f)));

        } // AppRunning loop

    } // Game Dll found

    CloseVulkan();

    if (Vulkan.Lib)
    {
        FreeLibrary(Vulkan.Lib);
    }

    if (GameState.Lib)
    {
        FreeLibrary(GameState.Lib);
    }
    return 0;

}
