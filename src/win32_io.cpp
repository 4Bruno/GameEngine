#include <windows.h>
#include "game_platform.h"
#include "vulkan\vulkan_win32.h"

#define GAME_DLL "game.dll"
#define GAME_DLL_TEMP "game_temp.dll"

#define APP_NAME "Vulkan_hardcore"
#define APP_WINDOW_WIDTH  980
#define APP_WINDOW_HEIGHT 540 
#define APP_MOUSE_CONFINE_RECT_SIZE 50.0f

// windowsx.h
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))


#define QUAD_TO_MS(Q) Q.QuadPart * (1.0f / 1000.0f)


// https://docs.microsoft.com/en-us/windows/win32/dxtecharts/taking-advantage-of-high-dpi-mouse-movement?redirectedfrom=MSDN
// you can #include <hidusage.h> for these defines
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif


#define ON_WINDOW_RESIZE(name) i32 name(i32 Width,i32 Height)
typedef ON_WINDOW_RESIZE(on_window_resize);

extern b32 GlobalAppRunning;
extern b32 AllowMouseConfinement;
extern on_window_resize * GraphicsOnWindowResize;

/* BEGIN of vulkan OS calls */
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


/* END of vulkan OS calls */


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

inline b32
Win32HasFileBeenModified(FILETIME LastModified, char * Filename)
{
    FILETIME CurrentTime = Win32GetLastWriteTime(Filename);
    
    b32 Result = (CompareFileTime(&CurrentTime, &LastModified) != 0);

    return Result;
}

inline b32
Win32DeleteFile(const char * Filename)
{
    b32 Result = DeleteFile(Filename);
    return Result;
}

PLATFORM_CLOSE_HANDLE(Win32CloseFile)
{
    if (OpenFileResult.Handle)
    {
        CloseHandle(OpenFileResult.Handle);
    }
}

inline b32
Win32FileExists(const char * Filename)
{
    HANDLE Hnd = 
        CreateFileA(Filename, GENERIC_WRITE,FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    b32 Result = (Hnd != INVALID_HANDLE_VALUE);
    if (Result)
    {
        platform_open_file_result OpenFileResult;
        OpenFileResult.Handle  = Hnd; // Void * Handle;
        Win32CloseFile(OpenFileResult);
    }
    return Result;
}
inline b32
Win32CopyFile(const char * Source, const char * Destination)
{
    BOOL Result = CopyFile(Source,Destination, false);
    return (b32)Result;
}


platform_open_file_result
Win32OpenFile(const char * Filepath)
{
    platform_open_file_result Result = {};

    HANDLE FileHandle = CreateFileA(Filepath,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL ,0);

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

PLATFORM_READ_HANDLE(Win32ReadFile)
{

    DWORD BytesRead;
    OVERLAPPED Overlapped = {};

    Overlapped.Offset = Offset;
    Overlapped.OffsetHigh = 0;

    if (!ReadFile(OpenFileResult.Handle, Buffer, Size, &BytesRead, &Overlapped) ||
            (Size != BytesRead))
    {
        return false;
    }

    return true;
}

PLATFORM_OPEN_FILE_READ_ONLY(Win32OpenFileReadOnly)
{
    platform_open_file_result Result = {};

    // FILE_FLAG_OVERLAPPED supports setting offset in OVERLAPPED struct
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

struct win32_frame_timer
{
    LARGE_INTEGER PerfFreq;
    LARGE_INTEGER BeginningOfTime;

    LARGE_INTEGER FrameStartTime;
    r32 LastFrames[100];
    i32 LastFramesIndex;
    r32 AvgFrameRate;
};


win32_frame_timer
NewFrameTimer()
{
    win32_frame_timer Timer;
    QueryPerformanceFrequency(&Timer.PerfFreq);
    QueryPerformanceCounter(&Timer.BeginningOfTime);

    // http://www.geisswerks.com/ryan/FAQS/timing.html
    // Sleep will do granular scheduling up to 1ms
    timeBeginPeriod(1);

    return Timer;
}

void
EndFrameTimer()
{
    timeEndPeriod(1);
}

void
WaitForFrameTimer(win32_frame_timer * FrameTimer, r32 ExpectedMillisecondsPerFrame)
{
    LARGE_INTEGER TimeFrameElapsed = 
        Win32QueryPerformanceDiff(Win32QueryPerformance(), FrameTimer->FrameStartTime, FrameTimer->PerfFreq);

    r32 TimeFrameRemaining = ExpectedMillisecondsPerFrame - QUAD_TO_MS(TimeFrameElapsed);

    FrameTimer->LastFrames[++FrameTimer->LastFramesIndex % ArrayCount(FrameTimer->LastFrames)] = QUAD_TO_MS(TimeFrameElapsed);

    r32 AvgFrameTime = 0.0f;
    u32 TotalFramesRecorded = ArrayCount(FrameTimer->LastFrames);
    for (u32 i = 15;
             i > 0;
             --i)
    {
        u32 Index = (FrameTimer->LastFramesIndex - i) % TotalFramesRecorded;
        AvgFrameTime += FrameTimer->LastFrames[Index];
    }

    FrameTimer->AvgFrameRate = (AvgFrameTime * (1.0f / 15.0f));

    //Log("Time frame remaining %f\n",TimeFrameRemaining);

    if (TimeFrameRemaining > 1.0f)
    {
        TimeFrameElapsed = Win32QueryPerformanceDiff(Win32QueryPerformance(), FrameTimer->FrameStartTime, FrameTimer->PerfFreq);
        TimeFrameRemaining = ExpectedMillisecondsPerFrame - QUAD_TO_MS(TimeFrameElapsed);
        while (TimeFrameRemaining > 1.0f)
        {
            Sleep((DWORD)TimeFrameRemaining);
            TimeFrameElapsed = Win32QueryPerformanceDiff(Win32QueryPerformance(), FrameTimer->FrameStartTime, FrameTimer->PerfFreq);
            TimeFrameRemaining = ExpectedMillisecondsPerFrame - QUAD_TO_MS(TimeFrameElapsed);
        }
    }
}

r32
CalculateTimeElapsed(win32_frame_timer * FrameTimer)
{
    LARGE_INTEGER TimeElapsed = 
        Win32QueryPerformanceDiff(Win32QueryPerformance(), FrameTimer->BeginningOfTime, FrameTimer->PerfFreq);
    r32 Delta = (r32)(TimeElapsed.QuadPart * (1.0f / 1000000.0f));

    return Delta;
}

void
BeginFrameTimer(win32_frame_timer * FrameTimer)
{
    FrameTimer->FrameStartTime = Win32QueryPerformance();
}


void
ResetInput(game_input * Input)
{
    for (u32 ButtonIndex = 0;
            ButtonIndex < ArrayCount(Input->Controller.Buttons);
            ++ButtonIndex)
    {
        game_button * Button = Input->Controller.Buttons + ButtonIndex;
        Button->WasPressed = Button->IsPressed;
    }
    for (u32 ButtonIndex = 0;
            ButtonIndex < ArrayCount(Input->Controller.Numbers);
            ++ButtonIndex)
    {
        game_button * Button = Input->Controller.Numbers + ButtonIndex;
        Button->WasPressed = Button->IsPressed;
    }
}

struct win32_window_dim
{
    i32 Width, Height;
};

win32_window_dim
Win32GetWindowSize(HWND WindowHandle)
{
    win32_window_dim Dim;

    RECT Rect;
    GetWindowRect(WindowHandle, &Rect);
    Dim.Height = Rect.bottom - Rect.top;
    Dim.Width  = Rect.right - Rect.left;

    return Dim;

}

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

void *
Win32AllocateMemory(u32 Size)
{
    void * Addr = VirtualAlloc(0, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    return Addr;
}
void
Win32DeallocMemory(void * Addr)
{
    if (Addr)
    {
        VirtualFree(Addr, 0, MEM_RELEASE);
    }
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
            if (GraphicsOnWindowResize(Width,Height))
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



