#include <Windows.h>
#include "game_platform.h"
#include "win32_io.cpp"
#include "vulkan\vulkan_win32.h"
#include "vulkan_initializer.h"
#include "win32_threading.cpp"

// https://docs.microsoft.com/en-us/windows/win32/dxtecharts/taking-advantage-of-high-dpi-mouse-movement?redirectedfrom=MSDN
// you can #include <hidusage.h> for these defines
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

#define GAME_DLL "game.dll"
#define GAME_DLL_TEMP "game_temp.dll"
#define SHADER_VERTEX_DEBUG "shaders\\triangle.vert"

#define APP_NAME "Vulkan_hardcore"
#define APP_WINDOW_WIDTH  980
#define APP_WINDOW_HEIGHT 540 
#define APP_MOUSE_CONFINE_RECT_SIZE 50.0f

// windowsx.h
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))

#define ENABLED_DEBUG_VULKAN_VALIDATION_LAYER 1

#define QUAD_TO_MS(Q) Q.QuadPart * (1.0f / 1000.0f)


global_variable b32  GlobalAppRunning = false;
global_variable RECT GlobalOldRectClip;
global_variable RECT GlobalNewRectClip;
global_variable b32 AllowMouseConfinement = false;


/* BEGIN of OS Specific calls to be replicated in other OS */

LRESULT CALLBACK 
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

b32
Win32BuildRelativePath(char * Buffer, const char * Filename)
{
    u32 Size = 255;
    char cd[255];
    DWORD cdSize= GetModuleFileNameA(0,&cd[0],Size);

    b32 Result = false;

    if (Result)
    {
        u32 c = 0;
        for (;
             c < cdSize;
             ++c)
        {
            Buffer[c] = cd[c];
        }
        for (u32 i = 0;
             ((c + i < Size) && Filename[i]);
             ++i)
        {
            Buffer[c++] = Filename[i];
        }
        Result = true;
    }
    return Result;
}

inline LARGE_INTEGER
Win32QueryPerformance()
{
    LARGE_INTEGER Performance;
    QueryPerformanceCounter(&Performance);
    return Performance;
}

/* Returns us microseconds 10^-6 */
inline LARGE_INTEGER
Win32QueryPerformanceDiff(LARGE_INTEGER TimeEnd,LARGE_INTEGER TimeStart,LARGE_INTEGER PerformanceFrequency)
{
    TimeEnd.QuadPart = TimeEnd.QuadPart - TimeStart.QuadPart;
    TimeEnd.QuadPart *= 1000000;
    TimeEnd.QuadPart /= PerformanceFrequency.QuadPart;
    return TimeEnd;
}

void
Win32RegisterRawInput(HWND WindowHandle)
{
    RAWINPUTDEVICE Rid[1];
    Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC; 
    Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE; 
    Rid[0].dwFlags = RIDEV_INPUTSINK;   
    Rid[0].hwndTarget = WindowHandle;
    RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
}

inline void
Win32ReleaseCursor()
{
    ClipCursor(0); 
}

inline void
Win32ConfineCursor(HWND WindowHandle)
{
    RECT Rect;
    GetWindowRect(WindowHandle, &Rect); 

    LONG cx, cy;
    cx = ((Rect.right - Rect.left) / 2 ) + Rect.left;
    cy = ((Rect.bottom - Rect.top) / 2 ) + Rect.top ;

    //SetCursorPos(cx + 1,cy + 1);

    i32 ConfineBoxSize = (i32)APP_MOUSE_CONFINE_RECT_SIZE;
    Rect.left   = cx - ConfineBoxSize;
    Rect.right  = cx + ConfineBoxSize;
    Rect.top    = cy - ConfineBoxSize;
    Rect.bottom = cy + ConfineBoxSize;

    ClipCursor(0);
    ClipCursor(&Rect);
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
/* END of OS Specific calls to be replicated in other OS */


/* BEGIN of vulkan OS calls */
struct vulkan_window_data
{
    HINSTANCE hInstance;
    HWND      WindowHandle;
};

struct vulkan_handler
{
    HINSTANCE Lib;
    FARPROC GetInstanceProcAddr;
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

/* END of vulkan OS calls */

void
UpdateGameButton(game_button * Button, b32 IsPressed)
{
    Button->WasPressed = !IsPressed;
    Button->IsPressed = IsPressed;
    ++Button->Transitions;
}


void
HandleInput(game_controller * Controller,HWND WindowHandle, i32 Width, i32 Height)
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
                u32 VKCode = (u32)msg.wParam;

                b32 IsAltPressed = (msg.lParam & (1 << 29));
                // https://docs.microsoft.com/en-us/windows/win32/inputdev/using-keyboard-input
                b32 IsShiftPressed = (GetKeyState(VK_SHIFT) & (1 << 15));
                b32 IsPressed = (msg.lParam & (1UL << 31)) == 0;
                b32 WasPressed = (msg.lParam & (1 << 30)) != 0;

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
                    else if (VKCode == VK_SPACE)
                    {
                        UpdateGameButton(&Controller->Space,IsPressed);
                    }
                    else if (VKCode == 'L')
                    {
                        UpdateGameButton(&Controller->Lock, IsPressed);
                    }
                    else if (VKCode == 'R')
                    {
                        UpdateGameButton(&Controller->R, IsPressed);
                    }
                    else if (VKCode >= '0' && VKCode <= '9')
                    {
                        UpdateGameButton(&Controller->Numbers[VKCode - '0'], IsPressed);
                    }
                }
            } break;
#if 1
            case WM_MOUSEMOVE:
            {
                RECT Rect;
                GetWindowRect(WindowHandle, &Rect);
                i32 yHeight =  Rect.bottom - Rect.top;
                i32 xWidth = Rect.right - Rect.left;
                i32 xPos = (i32)GET_X_LPARAM(msg.lParam) + 1;
                i32 yPos = (i32)GET_Y_LPARAM(msg.lParam) + 1; 
                r32 HalfWidth = ((r32)(Width) * 0.5f);
                r32 HalfHeight = ((r32)(Height) * 0.5f);
                Controller->MouseX = ((r32)xPos - HalfWidth)  / APP_MOUSE_CONFINE_RECT_SIZE;
                Controller->MouseY = ((r32)yPos - HalfHeight) / APP_MOUSE_CONFINE_RECT_SIZE;
            } break;
#endif
            case WM_INPUT:
            {
                UINT dwSize = sizeof(RAWINPUT);
                static BYTE lpb[sizeof(RAWINPUT)];

                GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

                RAWINPUT* raw = (RAWINPUT*)lpb;

                if (raw->header.dwType == RIM_TYPEMOUSE) 
                {
                    Controller->RelMouseX = raw->data.mouse.lLastX;
                    // Flip Y
                    Controller->RelMouseY = -raw->data.mouse.lLastY;
                    
                } 
            } break;
            case WM_RBUTTONDOWN:
            {
                UpdateGameButton(&Controller->MouseRight, true);
            }break;
            case WM_RBUTTONUP:
            {
                UpdateGameButton(&Controller->MouseRight, false);
            }break;

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
        case WM_SIZING:
        {
            RECT * Rect = (RECT *)lParam;
            i32 Height =  Rect->bottom - Rect->top;
            i32 Width = Rect->right - Rect->left;
            if ((Height < APP_WINDOW_HEIGHT))
            {
                Rect->bottom = Rect->top + APP_WINDOW_HEIGHT;
            }
            if ((Width < APP_WINDOW_WIDTH))
            {
                Rect->right = Rect->left + APP_WINDOW_WIDTH;
            }
        } break;
        case WM_SIZE:
        {
            RECT Rect;
            GetWindowRect(hWnd, &Rect);
            i32 Height =  Rect.bottom - Rect.top;
            i32 Width = Rect.right - Rect.left;
            if (VulkanOnWindowResize(Width,Height))
            {
                OutputDebugStringA("Error swap chain creation during window resize\n");
                Log("Error during swap chain creation\n");
                GlobalAppRunning = false;
            };
        } break;
        case WM_MOVE:
        {
            // Allow resizing window
            Win32ReleaseCursor();
        } break;
        case WM_ACTIVATE:
        {
            // Allow resizing window
            if (wParam == WA_INACTIVE)
            {
                AllowMouseConfinement = false;
                Win32ReleaseCursor();
            }
            else
            {
                AllowMouseConfinement = true;
            }
        } break;
        default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        };
    };
    return true;
}


struct game_state
{
    HMODULE                  Lib;
    game_update_and_render * pfnGameUpdateAndRender;
    FILETIME                 CurrentLibLastModified;
    FILETIME                 ShaderVertexLastModified;
};



b32
LoadGameDll(game_state * GameState)
{
    if (GameState->Lib)
    {
        FreeLibrary(GameState->Lib);
        GameState->Lib = 0;
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

    GameState->Lib = LoadLibrary(GAME_DLL); // HMODULE   Lib;

    if (GameState->Lib)
    {
        GameState->pfnGameUpdateAndRender = 
            (game_update_and_render *)GetProcAddress(GameState->Lib, "GameUpdateAndRender"); // game_update_and_render   pfnGameUpdateAndRender;
        GameState->CurrentLibLastModified = Win32GetLastWriteTime((char *)GAME_DLL_TEMP);
        if (!GameState->pfnGameUpdateAndRender)
        {
            FreeLibrary(GameState->Lib);
            GameState->Lib = 0;
        }
    }

    b32 Result = (GameState->Lib && GameState->pfnGameUpdateAndRender);

    return Result;
}




//int main( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
int main()
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
    CreateWorkQueue(&RenderWorkQueue, 1);
    /* ------------------------- THREADS ----------------------------------- */

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

    if (LoadGameDll(&GameState))
    {
        u32 PermanentMemorySize = Megabytes(30);
        void * PermanentMemory = VirtualAlloc(0, PermanentMemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        u32 TransientMemorySize = Gigabytes(1);
        void * TransientMemory = VirtualAlloc(0, TransientMemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        game_memory GameMemory     = {};

        GameMemory.DebugOpenFile       = Win32OpenFile;       // debug_open_file DebugOpenFile;
        GameMemory.DebugReadFile       = Win32ReadFile;       // debug_read_file DebugReadFile;
        GameMemory.DebugCloseFile      = Win32CloseFile;      // debug_close_file DebugCloseFile;
        GameMemory.PermanentMemory     = PermanentMemory;     // Void * PermanentMemory;
        GameMemory.PermanentMemorySize = PermanentMemorySize; // u32 PermanentMemorySize;
        GameMemory.TransientMemory     = TransientMemory;     // Void * TransientMemory;
        GameMemory.TransientMemorySize = TransientMemorySize; // u32 TransientMemorySize;

        GameMemory.AddWorkToWorkQueue    = AddWorkToQueue;
        GameMemory.CompleteWorkQueue     = CompleteWorkQueue;
        GameMemory.HighPriorityWorkQueue = &HighPriorityWorkQueue;
        GameMemory.LowPriorityWorkQueue  = &LowPriorityWorkQueue;
        GameMemory.RenderWorkQueue       = &RenderWorkQueue;

        /* ------------------------- END GAME MEMORY ------------------------- */

        i32 ExpectedFramesPerSecond = 30;
        r32 ExpectedMillisecondsPerFrame = (1.0f / (r32)ExpectedFramesPerSecond) * 1000.0f;
        GlobalAppRunning = true;
        game_input Input = {};
        Input.DtFrame = ExpectedMillisecondsPerFrame / 1000.0f;

        // http://www.geisswerks.com/ryan/FAQS/timing.html
        // Sleep will do granular scheduling up to 1ms
        timeBeginPeriod(1);

        //https://docs.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stampsp
        LARGE_INTEGER PerfFreq;
        LARGE_INTEGER StartingTime;
        QueryPerformanceFrequency(&PerfFreq);
        QueryPerformanceCounter(&StartingTime);

        Win32RegisterRawInput(WindowHandle);

        //b32 AllowMouseConfinement = false;

        GameState.ShaderVertexLastModified = Win32GetLastWriteTime((char *)SHADER_VERTEX_DEBUG);

#if DEBUG
        u64 GameCycles[100] = {};
        u32 CurrentCycle = 0;
        LARGE_INTEGER LastTimeSinceDebugLog;
        QueryPerformanceCounter(&LastTimeSinceDebugLog);
        debug_cycle DebugCycles[DEBUG_CYCLE_COUNT] = {};
        GameMemory.DebugCycle = &DebugCycles[0];
#endif
        // Main loop
        while (GlobalAppRunning)
        {
#if DEBUG
            if (Win32HasFileBeenModified(GameState.CurrentLibLastModified, (char *)GAME_DLL_TEMP) && Win32FileExists(GAME_DLL_TEMP))
            {
                VulkanWaitForDevices();
                if (!LoadGameDll(&GameState))
                {
                    GlobalAppRunning = false;
                    break;
                }
                Input.Reloaded = true;
            }
            else
            {
                Input.Reloaded = false;
            }
            if (Win32HasFileBeenModified(GameState.ShaderVertexLastModified, 
                                        (char *)SHADER_VERTEX_DEBUG) && Win32FileExists(SHADER_VERTEX_DEBUG))
            {
                VulkanWaitForDevices();
                Input.ShaderHasChanged = true;
                GameState.ShaderVertexLastModified = Win32GetLastWriteTime((char *)SHADER_VERTEX_DEBUG);
            }
#endif
            LARGE_INTEGER TimeElapsed = 
                Win32QueryPerformanceDiff(Win32QueryPerformance(), StartingTime, PerfFreq);
            Input.TimeElapsed = (r32)(TimeElapsed.QuadPart * (1.0f / 1000000.0f));
            LARGE_INTEGER TimeFrameStart = Win32QueryPerformance();

            for (u32 ButtonIndex = 0;
                        ButtonIndex < ArrayCount(Input.Controller.Buttons);
                        ++ButtonIndex)
            {
                game_button * Button = Input.Controller.Buttons + ButtonIndex;
                Button->WasPressed = Button->IsPressed;
            }
            for (u32 ButtonIndex = 0;
                        ButtonIndex < ArrayCount(Input.Controller.Numbers);
                        ++ButtonIndex)
            {
                game_button * Button = Input.Controller.Numbers + ButtonIndex;
                Button->WasPressed = Button->IsPressed;
            }

            if (AllowMouseConfinement)
            {
                Win32ConfineCursor(WindowHandle);
            }

            // TODO: function call
            RECT Rect;
            GetWindowRect(WindowHandle, &Rect);
            i32 Height =  Rect.bottom - Rect.top;
            i32 Width = Rect.right - Rect.left;

            //Log("Left: %i Bottom: %i\n",Rect.left, Rect.bottom - Rect.top);

            Input.Controller.RelMouseX = 0;
            Input.Controller.RelMouseY = 0;

            HandleInput(&Input.Controller,WindowHandle, Width, Height);

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
            GameState.pfnGameUpdateAndRender(&GameMemory,&Input,Width, Height);
#endif

            LARGE_INTEGER TimeFrameElapsed = 
                Win32QueryPerformanceDiff(Win32QueryPerformance(), TimeFrameStart, PerfFreq);
            r32 TimeFrameRemaining = ExpectedMillisecondsPerFrame - QUAD_TO_MS(TimeFrameElapsed);

            //Log("Time frame remaining %f\n",TimeFrameRemaining);

            if (TimeFrameRemaining > 1.0f)
            {
                TimeFrameElapsed = Win32QueryPerformanceDiff(Win32QueryPerformance(), TimeFrameStart, PerfFreq);
                TimeFrameRemaining = ExpectedMillisecondsPerFrame - QUAD_TO_MS(TimeFrameElapsed);
                while (TimeFrameRemaining > 1.0f)
                {
                    Sleep((DWORD)TimeFrameRemaining);
                    TimeFrameElapsed = Win32QueryPerformanceDiff(Win32QueryPerformance(), TimeFrameStart, PerfFreq);
                    TimeFrameRemaining = ExpectedMillisecondsPerFrame - QUAD_TO_MS(TimeFrameElapsed);
                }
            }

            //Log("Time frame work %f\n",(r32)(TimeFrameElapsed.QuadPart * (1.0f / 1000.0f)));

#if DEBUG
            LARGE_INTEGER DebugTimeEnd = Win32QueryPerformance();
            LARGE_INTEGER DebugTimeElapsed = 
                Win32QueryPerformanceDiff(DebugTimeEnd, LastTimeSinceDebugLog, PerfFreq);

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
                    LastTimeSinceDebugLog = DebugTimeEnd;
                }
                Index = (Index + 1) & (DEBUG_CYCLE_HISTORY - 1);
                DebugCycles[DebugCycleIndex].RingIndex = Index;
                DebugCycles[DebugCycleIndex].NumberOfCalls[Index] = 0;
                DebugCycles[DebugCycleIndex].NumberOfCycles[Index] = 0;
            }

#endif

        } // AppRunning loop

        Win32ReleaseCursor();

        timeEndPeriod(1);

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
