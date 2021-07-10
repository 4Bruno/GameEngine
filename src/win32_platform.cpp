#include <Windows.h>
#include "game_platform.h"
#include "vulkan\vulkan_win32.h"
#include "vulkan_initializer.h"
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

global_variable bool32  GlobalAppRunning = false;
global_variable RECT GlobalOldRectClip;
global_variable RECT GlobalNewRectClip;


/* BEGIN of OS Specific calls to be replicated in other OS */

LRESULT CALLBACK 
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

bool32
Win32BuildRelativePath(char * Buffer, const char * Filename)
{
    uint32 Size = 255;
    char cd[255];
    DWORD cdSize= GetModuleFileNameA(0,&cd[0],Size);

    bool32 Result = false;

    if (Result)
    {
        uint32 c = 0;
        for (;
             c < cdSize;
             ++c)
        {
            Buffer[c] = cd[c];
        }
        for (uint32 i = 0;
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

    int32 ConfineBoxSize = (int32)APP_MOUSE_CONFINE_RECT_SIZE;
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

inline FILETIME
Win32GetLastWriteTime(char * Filename)
{
    FILETIME LastWriteTime = {};
    
    WIN32_FILE_ATTRIBUTE_DATA Data;

    if(GetFileAttributesExA(Filename, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }
    
    return LastWriteTime;
}

inline bool32
Win32HasFileBeenModified(FILETIME LastModified, char * Filename)
{
    FILETIME CurrentTime = Win32GetLastWriteTime(Filename);
    
    bool32 Result = (CompareFileTime(&CurrentTime, &LastModified) != 0);

    return Result;
}

inline bool32
Win32DeleteFile(const char * Filename)
{
    bool32 Result = DeleteFile(Filename);
    return Result;
}

void
Win32CloseFile(platform_open_file_result Result)
{
    if (Result.Handle)
    {
        CloseHandle(Result.Handle);
    }
}
inline bool32
Win32FileExists(const char * Filename)
{
    HANDLE Hnd = 
        CreateFileA(Filename, GENERIC_WRITE,FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    bool32 Result = (Hnd != INVALID_HANDLE_VALUE);
    if (Result)
    {
        platform_open_file_result OpenFileResult;
        OpenFileResult.Handle  = Hnd; // Void * Handle;
        Win32CloseFile(OpenFileResult);
    }
    return Result;
}
inline bool32
Win32CopyFile(const char * Source, const char * Destination)
{
    BOOL Result = CopyFile(Source,Destination, false);
    return (bool32)Result;
}


platform_open_file_result
Win32OpenFile(const char * Filepath)
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
Win32ReadFile(platform_open_file_result OpenFileResult, void * Buffer)
{

    DWORD BytesRead;

    if (!ReadFile(OpenFileResult.Handle, Buffer, OpenFileResult.Size, &BytesRead, 0) ||
            (OpenFileResult.Size != BytesRead))
    {
        return false;
    }

    return true;

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
UpdateGameButton(game_button * Button, bool32 IsPressed)
{
    Button->WasPressed = !IsPressed;
    Button->IsPressed = IsPressed;
    ++Button->Transitions;
}


void
HandleInput(game_controller * Controller,HWND WindowHandle, int32 Width, int32 Height)
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
                }
            } break;
#if 1
            case WM_MOUSEMOVE:
            {
                RECT Rect;
                GetWindowRect(WindowHandle, &Rect);
                int32 yHeight =  Rect.bottom - Rect.top;
                int32 xWidth = Rect.right - Rect.left;
                int32 xPos = (int32)GET_X_LPARAM(msg.lParam) + 1;
                int32 yPos = (int32)GET_Y_LPARAM(msg.lParam) + 1; 
                real32 HalfWidth = ((real32)(Width) * 0.5f);
                real32 HalfHeight = ((real32)(Height) * 0.5f);
                Controller->MouseX = ((real32)xPos - HalfWidth)  / APP_MOUSE_CONFINE_RECT_SIZE;
                Controller->MouseY = ((real32)yPos - HalfHeight) / APP_MOUSE_CONFINE_RECT_SIZE;
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
            int32 Height =  Rect->bottom - Rect->top;
            int32 Width = Rect->right - Rect->left;
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
            int32 Height =  Rect.bottom - Rect.top;
            int32 Width = Rect.right - Rect.left;
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



bool32
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

    bool32 Result = (GameState->Lib && GameState->pfnGameUpdateAndRender);

    return Result;
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

    if (LoadGameDll(&GameState))
    {
        uint32 PermanentMemorySize = Megabytes(30);
        void * PermanentMemory = VirtualAlloc(0, PermanentMemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        uint32 TransientMemorySize = Gigabytes(1);
        void * TransientMemory = VirtualAlloc(0, TransientMemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        game_memory GameMemory     = {};

        GameMemory.DebugOpenFile       = Win32OpenFile;       // debug_open_file DebugOpenFile;
        GameMemory.DebugReadFile       = Win32ReadFile;       // debug_read_file DebugReadFile;
        GameMemory.DebugCloseFile      = Win32CloseFile;      // debug_close_file DebugCloseFile;
        GameMemory.PermanentMemory     = PermanentMemory;     // Void * PermanentMemory;
        GameMemory.PermanentMemorySize = PermanentMemorySize; // uint32 PermanentMemorySize;
        GameMemory.TransientMemory     = TransientMemory;     // Void * TransientMemory;
        GameMemory.TransientMemorySize = TransientMemorySize; // uint32 TransientMemorySize;
        /* ------------------------- END GAME MEMORY ------------------------- */

        int32 ExpectedFramesPerSecond = 30;
        real32 ExpectedMillisecondsPerFrame = (1.0f / (real32)ExpectedFramesPerSecond) * 1000.0f;
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

        bool32 AllowMouseConfinement = false;

        GameState.ShaderVertexLastModified = Win32GetLastWriteTime((char *)SHADER_VERTEX_DEBUG);

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
            Input.TimeElapsed = (real32)(TimeElapsed.QuadPart * (1.0f / 1000000.0f));
            LARGE_INTEGER TimeFrameStart = Win32QueryPerformance();

            for (uint32 ButtonIndex = 0;
                        ButtonIndex < ArrayCount(Input.Controller.Buttons);
                        ++ButtonIndex)
            {
                game_button * Button = Input.Controller.Buttons + ButtonIndex;
                Button->WasPressed = Button->IsPressed;
            }

            if (AllowMouseConfinement)
            {
                Win32ConfineCursor(WindowHandle);
            }

            // TODO: function call
            RECT Rect;
            GetWindowRect(WindowHandle, &Rect);
            int32 Height =  Rect.bottom - Rect.top;
            int32 Width = Rect.right - Rect.left;

            //Log("Left: %i Bottom: %i\n",Rect.left, Rect.bottom - Rect.top);

            Input.Controller.RelMouseX = 0;
            Input.Controller.RelMouseY = 0;

            HandleInput(&Input.Controller,WindowHandle, Width, Height);

            if (Input.Controller.Lock.IsPressed && !Input.Controller.Lock.WasPressed)
            {
                if (AllowMouseConfinement) Win32ReleaseCursor();
                AllowMouseConfinement = !AllowMouseConfinement;
            }

            GameState.pfnGameUpdateAndRender(&GameMemory,&Input,Width, Height);

#define QUAD_TO_MS(Q) Q.QuadPart * (1.0f / 1000.0f)

            LARGE_INTEGER TimeFrameElapsed = 
                Win32QueryPerformanceDiff(Win32QueryPerformance(), TimeFrameStart, PerfFreq);
            real32 TimeFrameRemaining = ExpectedMillisecondsPerFrame - QUAD_TO_MS(TimeFrameElapsed);

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

            //Log("Time frame work %f\n",(real32)(TimeFrameElapsed.QuadPart * (1.0f / 1000.0f)));

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
