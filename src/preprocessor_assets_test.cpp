#include "preprocessor_assets.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "vulkan\vulkan_win32.h"
#include "vulkan_initializer.cpp"
#include "Quaternion.cpp"
#include "game_world.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"


#define APP_NAME "preprocessor_test"
#define APP_WINDOW_WIDTH  980
#define APP_WINDOW_HEIGHT 540 
#define ENABLED_DEBUG_VULKAN_VALIDATION_LAYER 1

// windowsx.h
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#define QUAD_TO_MS(Q) Q.QuadPart * (1.0f / 1000.0f)

global_variable b32 GlobalAppRunning      = true;
global_variable r32 GlobalFontPixelSize   = 14.0f;
global_variable platform_api PlatformAPI  = {};


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
UpdateGameButton(game_button * Button, b32 IsPressed)
{
    Button->WasPressed = !IsPressed;
    Button->IsPressed = IsPressed;
    ++Button->Transitions;
}

global_variable char LastKeyCharPressed = 'b';

inline b32 
IsLetter(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
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

                    if (IsLetter((char)VKCode))
                    {
                        LastKeyCharPressed = VKCode - ('A' - 'a');
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
                Controller->MouseX = ((r32)xPos - HalfWidth)  / 1.0f;
                Controller->MouseY = ((r32)yPos - HalfHeight) / 1.0f;
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
            if (OnWindowResize(Width,Height))
            {
                OutputDebugStringA("Error swap chain creation during window resize\n");
                Log("Error during swap chain creation\n");
                GlobalAppRunning = false;
            };
        } break;
        case WM_MOVE:
        {
        } break;
        case WM_ACTIVATE:
        {
        } break;
        default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        };
    };
    return true;
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

graphics_platform_window_result
InitVulkan(vulkan_window_data * VulkanWindowData)
{
    graphics_platform_window_result Result = {};

    HINSTANCE VulkanLib = LoadLibrary("vulkan-1.dll"); // HINSTANCE   Lib;

    if (VulkanLib)
    {
        FARPROC pfnVulkanGetInstanceProcAddr = GetProcAddress(VulkanLib, "vkGetInstanceProcAddr"); // FARPROC   GetInstanceProcAddr;
                                                                                                   //
        if (pfnVulkanGetInstanceProcAddr)
        {
            Result.Window.pfnVulkanCreateSurface      = VulkanCreateSurface; // vulkan_create_surface   * pfnVulkanCreateSurface;
            Result.Window.SurfaceData                 = (void *)VulkanWindowData; // Void * SurfaceData;
            Result.Window.VkKHROSSurfaceExtensionName = VK_KHR_WIN32_SURFACE_EXTENSION_NAME; // Char_S * VkKHROSSurfaceExtensionName;
            Result.Window.OSSurfaceFuncName           = "vkCreateWin32SurfaceKHR"; // Char_S * OSSurfaceFuncName;
                                                                                   //
            if (InitializeVulkan(APP_WINDOW_WIDTH,APP_WINDOW_HEIGHT,
                        Result.Window,
                        ENABLED_DEBUG_VULKAN_VALIDATION_LAYER,
                        (PFN_vkGetInstanceProcAddr)pfnVulkanGetInstanceProcAddr))
            {
                Log("Failed to initialize Vulkan\n");
            }
            else
            {
                Result.Success = true;
            }
        }
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

internal u32
Win32RewindFile(HANDLE handle, u32 Offset = 0)
{
    DWORD Result = SetFilePointer(handle, Offset, NULL,FILE_BEGIN);
    return Result;
}

void
TestAssetsImport(assets_handler * Assets)
{
    for (u32 AssetTypeIndex = 0;
             AssetTypeIndex < Assets->AssetTypeCount;
             ++AssetTypeIndex)
    {
        bin_asset_type * AssetType = Assets->AssetType + AssetTypeIndex;

        for (u32 AssetIndex = AssetType->Begin;
                AssetIndex < AssetType->End;
                ++AssetIndex)
        {
            bin_asset * Asset = Assets->Assets + AssetIndex;

            switch (Asset->FileType)
            {
                case asset_file_type_mesh:
                    {
                        vertex_point * Vertices = (vertex_point *)malloc(Asset->Mesh.SizeVertices);
                        PlatformAPI.ReadHandle(Assets->PlatformHandle, Vertices, Asset->Mesh.SizeVertices, Asset->DataBeginOffset);

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

                        free(Vertices);

                    } break;

                case asset_file_type_texture:
                    {
#if 0
                        bin_text * Text = &Asset->Text;
                        u32 TextSize = Text->Height * Text->Width * Text->Channels;
                        void * Raw = malloc(TextSize);
                        ReadFromFile(Assets->PlatformHandle, Raw, TextSize);
                        free(Raw);
#endif
                    } break;

                case asset_file_type_unknown: break;
                case asset_file_type_sound: break;
                case asset_file_type_shader: 
                {
                    void * Raw = malloc(Asset->Size);
                    PlatformAPI.ReadHandle(Assets->PlatformHandle, Raw, Asset->Size, Asset->DataBeginOffset);
                    i32 Shader = CreateShaderModule(Raw, Asset->Size);
                    Logn("%s to load shader %i", (Shader >= 0) ? "Success" : "Unsuccessful",AssetIndex);

                    free(Raw);
                } break;
                case asset_file_type_shader_vertex: break;
                case asset_file_type_shader_fragment: break;
                case asset_file_type_shader_geometry: break;
                case asset_file_type_mesh_material: break;
            };
        }
    }
}

inline void
InitArena(memory_arena * Arena,u8 * BaseAddr, u32 MaxSize)
{
    Arena->MaxSize = MaxSize;
    Arena->CurrentSize = 0;
    Arena->Base = BaseAddr;
}

void
InitArena(memory_arena * Arena,void * BaseAddr, u32 MaxSize)
{
    InitArena(Arena,(u8 *)BaseAddr, MaxSize);
}
global_variable i32 ttf_tex  = 0;


inline b32
IsEOF(char c)
{
    return (c == 10 || c == 13);
}

struct char_render_info
{
    char c;
    r32 OffsetY;
};
struct font_texture_info
{
    i32 GPUTextureIndex;
    char_render_info CharRenderInfo['~' - ' ' + 1];
    r32 Ascent;
    i32 PixelSize;
};

#define LOAD_ASSET_POST_PROCESS(name) void name(void * Data)
typedef LOAD_ASSET_POST_PROCESS(load_asset_post_process);



void
PushDraw_(renderer * Renderer, game_asset * Mesh, m4 * Model, i32 Text, i32 SampleOffsetX, i32 SampleOffsetY)
{
    if (Renderer->Capacity > Renderer->Ocuppancy)
    {
        render_command * Command = Renderer->Commands + Renderer->Ocuppancy++;
        Command->Mesh.OffsetVertices  = 0;
        Command->Mesh.OffsetIndices   = 0;
        Command->Text.ID = Text;
        Command->Text.SampleOffsetX = SampleOffsetX;
        Command->Text.SampleOffsetY = SampleOffsetY;
        Command->Model = *Model;
    }
}

void
PushDrawText(renderer * Renderer, assets_handler * Assets, font_type FontType, r32 X, r32 Y, char * Text)
{
    game_asset Quad = GetMesh(Assets,game_asset_type_mesh_shape , asset_tag_quad);
    game_asset Font = GetFont(Assets,FontType);

    if (!Quad.Asset) LoadAsset(Assets, &Quad, false);
    if (!Font.Asset) LoadAsset(Assets, &Font, false);

    if (Quad.Asset && Font.Asset)
    {
        i32 len = strlen(Text);

        m4 Model {};
        r32 OffsetX = 0.0f;
        for (char * c = Text; *c != 0; ++c)
        {
            i32 TextSample = *c  - ' ';
            i32 TextOffsetX = (r32)(TextSample % 10);
            i32 TextOffsetY = (r32)(TextSample / 10);
            Translate(Model, V3(X + OffsetX, Y, 0.0f));
            PushDraw_(Renderer, &Quad, &Model, 0, TextOffsetX, TextOffsetY);
            OffsetX += Font.Asset->Font.Height;
        }
    }
}

void
RenderScene(renderer * Renderer, assets_handler * Assets)
{
    u32 InstanceCount = Renderer->Ocuppancy;

    gpu_memory_mapping_result ObjectsMapResult = 
        BeginObjectMapping(InstanceCount);

    if (!ObjectsMapResult.Success)
    {
        Assert(0);
        return;
    }

    GPUObjectData * ObjectData = (GPUObjectData *)ObjectsMapResult.BeginAddress;

    for (u32 ObjectIndex = 0;
            ObjectIndex < InstanceCount;
            ++ObjectIndex)
    {
        render_command * Command = Renderer->Commands + ObjectIndex;
        *ObjectData = {};
        ObjectData->Color = V4(1.0f,1.0f,1.0f,0);
        ObjectData->TextSample = { (r32)Command->Text.SampleOffsetX, (r32)Command->Text.SampleOffsetY };
        ObjectData->ImageIndex = Command->Text.ID;

        ++ObjectData;
    }

    EndObjectsArena();

#if 0
    for (u32 ObjectIndex = 0;
            ObjectIndex < InstanceCount;
            ++ObjectIndex)
    {
        render_command * Command = Renderer->Commands + ObjectIndex;
        
        if (mesh changes or pipeline changes...)
        RenderPushMeshIndexed(InstanceCount, 6, Command->Mesh.OffsetVertices, Command->Mesh.OffsetIndices);
    }
#else
    render_command * Command = Renderer->Commands + 0;
    RenderPushMeshIndexed(InstanceCount, 6, Command->Mesh.OffsetVertices, Command->Mesh.OffsetIndices);
#endif
}

void
GetDimBoxForCharInRange(stbtt_fontinfo * Font,r32 ScaleX, r32 ScaleY, i32 Start, i32 End, i32 * MaxW, i32 * MaxH)
{
    Assert(End >= Start);
    for (i32 c = Start; c <= End; ++c)
    {
        int ix0, iy0, ix1, iy1;
        stbtt_GetCodepointBitmapBox(Font, c, ScaleX, ScaleY, &ix0, &iy0, &ix1, &iy1);
        i32 W = (ix1 - ix0);
        i32 H = (iy1 - iy0);
        *MaxW = (W > *MaxW) ? W : *MaxW;
        *MaxH = (W > *MaxH) ? W : *MaxH;
    }
}

font_texture_info
CreateFontTexture(r32 PixelHeight)
{

    font_texture_info FontTextureInfo = {};
    FontTextureInfo.GPUTextureIndex = -1;

    DWORD BytesRead = 0;
    platform_open_file_result file_ttf = PlatformAPI.OpenFileReadOnly("c:/windows/fonts/times.ttf");

    if (!file_ttf.Success) return FontTextureInfo;

    void * buffer = malloc(file_ttf.Size);

    ReadFile(file_ttf.Handle, buffer, file_ttf.Size, &BytesRead, NULL);
    Assert(BytesRead == file_ttf.Size);

    stbtt_fontinfo Font;
    stbtt_InitFont(&Font, (u8 *)buffer, stbtt_GetFontOffsetForIndex((u8 *)buffer,0));

    r32 TextHeight = PixelHeight;
    r32 WidthOverHeight = (r32)GlobalVulkan.WindowExtension.width / (r32)GlobalVulkan.WindowExtension.height;
    i32 TotalWidth = 0;
    i32 TotalHeight = 0;

    r32 ScaleY = stbtt_ScaleForPixelHeight(&Font, TextHeight);
    r32 ScaleX = ScaleY;
    i32 Width ,Height ,XOffset ,YOffset;

    i32 MaxWidth = 0;
    i32 MaxHeight = 0;


#if 0
    i32 CountChars = 0;
    i32 CharsRanges[][2] = {
        {'A','Z'},
        {'a','z'},
        {'0','9'}
    };
#else
    i32 CountChars = 0;
    i32 CharsRanges[][2] = {
        {' ','~'},
    };
#endif

    for (u32 i = 0; i < ArrayCount(CharsRanges); ++i)
    {
        i32 Start = CharsRanges[i][0];
        i32 End = CharsRanges[i][1];
        GetDimBoxForCharInRange(&Font,ScaleX, ScaleY, Start, End, &MaxWidth, &MaxHeight);
        ++CountChars;
    }

    MaxWidth = NextPowerOf2(MaxWidth);
    MaxHeight = NextPowerOf2(MaxHeight);

    Logn("MaxWidth: %i MaxHeight:%i CountChars:%i PixelHeight:%f", MaxWidth, MaxHeight, CountChars, TextHeight);

    i32 AtlasStride = 10;
    TotalWidth = MaxWidth * AtlasStride;
    TotalHeight = MaxHeight * AtlasStride;

    Logn("TotalWidth: %i TotalHeight:%i CharsPerRow:%i", TotalWidth, TotalHeight, AtlasStride);

    Assert(SQR(AtlasStride) >= CountChars);

    u8 * AtlastBuffer = (u8 *)malloc(TotalWidth * TotalHeight * 1);
    RtlZeroMemory(AtlastBuffer, TotalWidth * TotalHeight * 1);

    u8 * Bitmap = (u8 *)malloc(MaxWidth * MaxHeight * 1);
    i32 CopiedBitmapCount = 0;
    for (u32 i = 0; i < ArrayCount(CharsRanges); ++i)
    {
        i32 Start = CharsRanges[i][0];
        i32 End = CharsRanges[i][1];
        for (i32 c = Start; c <= End; ++c)
        {
            int ix0, iy0, ix1, iy1;
            stbtt_GetCodepointBitmapBox(&Font, c, ScaleX, ScaleY, &ix0, &iy0, &ix1, &iy1);
            i32 AdvanceWidth, LeftSideBearing;
            stbtt_GetCodepointHMetrics(&Font, c, &AdvanceWidth, &LeftSideBearing);
            i32 W = (ix1 - ix0);
            i32 H = (iy1 - iy0);
            FontTextureInfo.CharRenderInfo[c - ' '].c = c;
            FontTextureInfo.CharRenderInfo[c - ' '].OffsetY = (r32)iy0 / (r32)H;
            i32 PaddingX = (MaxWidth - W) / 2;
            //i32 PaddingY = (MaxHeight - H) / 2;
            i32 PaddingY = MaxHeight - H;
            i32 Row = (CopiedBitmapCount / AtlasStride);
            i32 Col = (CopiedBitmapCount % AtlasStride);
            stbtt_MakeCodepointBitmapSubpixel(&Font, Bitmap, W, H, MaxWidth, ScaleX, ScaleY,0.0f, 0.0f, c);
            u8 * Dst = AtlastBuffer + 
                       (Row * AtlasStride * MaxWidth * MaxHeight) + 
                       (Col * MaxWidth) + 
                       PaddingX + 
                       (PaddingY * AtlasStride * MaxWidth);
            u8 * Src = Bitmap;
            for (i32 Y = 0; Y < H; ++Y)
            {
                i32 OffsetY = Y * AtlasStride * MaxWidth;
                memcpy(Dst + OffsetY, Src, W);
                Src += MaxWidth;
            }
            CopiedBitmapCount += 1;
        }
    }
    free(Bitmap);

    FontTextureInfo.GPUTextureIndex = PushTextureData(AtlastBuffer, TotalWidth, TotalHeight, 1);
    i32 Ascent, Descent, Linegap;
    stbtt_GetFontVMetrics(&Font, &Ascent, &Descent, &Linegap);
    FontTextureInfo.Ascent = (Ascent * ScaleY) / MaxWidth;
    FontTextureInfo.PixelSize = MaxHeight;

    free(buffer);
    free(AtlastBuffer);

    return FontTextureInfo;
}

renderer * 
NewRenderer(memory_arena * Arena, u32 MaxRenderUnits)
{
    renderer * R = PushStruct(Arena, renderer);
    R->Commands = PushArray(Arena, MaxRenderUnits, render_command);
    R->Capacity = MaxRenderUnits;
    R->Ocuppancy = 0;
}

int
main()
{

    HINSTANCE hInstance = GetModuleHandle(0);
    HWND WindowHandle = Win32CreateWindow(hInstance);
    vulkan_window_data VulkanWindowData { hInstance, WindowHandle };

    if (INVALID_HANDLE_VALUE == WindowHandle)
    {
        Logn("Failed to create window handler for Win32");
        return 0;
    };

    graphics_platform_window_result VulkanInitResult = InitVulkan(&VulkanWindowData);

    if (!VulkanInitResult.Success)
    {
        return 0;
    }

    graphics_platform_window Window = VulkanInitResult.Window;

    u32 PermanentMemorySize = Megabytes(30);
    void * PermanentMemory = VirtualAlloc(0, PermanentMemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    memory_arena Arena = {};
    InitArena(&Arena, PermanentMemory, PermanentMemorySize);

    memory_arena AssetsArena = {};
    u32 AssetsArenaSize = Megabytes(10);
    void * AssetsArenaBegin = PushSize(&Arena,AssetsArenaSize);
    InitArena(&AssetsArena,AssetsArenaBegin,AssetsArenaSize);

    memory_arena RenderArena = {};
    u32 MaxRenderUnits = 2048;
    u32 RenderArenaSize = sizeof(renderer) + 
                          sizeof(render_command) * MaxRenderUnits;
    void * RenderArenaBegin = PushSize(&Arena,RenderArenaSize);
    InitArena(&RenderArena,RenderArenaBegin,RenderArenaSize);

    renderer * Renderer = NewRenderer(&RenderArena, MaxRenderUnits);

    i32 VS = LoadShaderVertex(Assets,asset_tag_noperspective);
    i32 FS = LoadShaderFragment(Assets,asset_tag_texturesampling);

    pipeline_creation_result Pipeline = CreatePipeline(VS, FS, polygon_mode_fill);

#if 1
    font_texture_info FontTexture = CreateFontTexture(62.0f);
#else
    i32 Texture = -1;
    asset_data Text = GetAsset(&Assets, "ground_stone_01.jpg");
    int x,y,n;
    int desired_channels = 4;
    stbi_uc * Image = stbi_load_from_memory((const unsigned char *)Text.Begin, Text.Size, &x, &y, &n, desired_channels);
    Texture = PushTextureData(Image, x, y, desired_channels);
    stbi_image_free(Image);
#endif

    game_input Input = {};
    i32 ExpectedFramesPerSecond = 30;
    r32 ExpectedMillisecondsPerFrame = (1.0f / (r32)ExpectedFramesPerSecond) * 1000.0f;
    Input.DtFrame = ExpectedMillisecondsPerFrame / 1000.0f;

    win32_frame_timer FrameTimer = NewFrameTimer();

    ma_result result;
    ma_engine engine;

    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        Logn("Failed to initialize audio engine.");
        return -1;
    }

    static char KeyPressed = LastKeyCharPressed;
    while (GlobalAppRunning)
    {
        BeginFrameTimer(&FrameTimer);

        win32_window_dim WinDim = Win32GetWindowSize(WindowHandle);

        Input.TimeElapsed = CalculateTimeElapsed(&FrameTimer);

        HandleInput(&Input.Controller,WindowHandle, WinDim.Width, WinDim.Height);

        if (Input.Controller.Up.IsPressed) 
        {
            GlobalFontPixelSize += 0.03f;
            Logn("GlobalFontPixelSize: %f", GlobalFontPixelSize);
        }
        if (Input.Controller.Down.IsPressed)
        { 
            GlobalFontPixelSize -= 0.03f;
            Logn("GlobalFontPixelSize: %f", GlobalFontPixelSize);
        }

        if (!GlobalWindowIsMinimized)
        {
            WaitForRender();

            v4 ClearColor = V4(0,0,0,1);

            RenderBeginPass(ClearColor);

            RenderSetPipeline(Pipeline.Pipeline);

            //DrawText(Pipeline.Pipeline,&FontTexture, "This is a test paco quality fernando, this and that. !!!", GPUVertexOffset, GPUIndicesOffset, Input.TimeElapsed);
            
            char buffer[30];
            //snprintf(buffer,30,"FPS: %f",FrameTimer.AvgFrameRate);
            if (LastKeyCharPressed != KeyPressed) 
            {
                static ma_sound SoundFiles['z' - 'a'] = {};

                KeyPressed = LastKeyCharPressed;
                snprintf(buffer,ArrayCount(buffer),"assets\\letter_%c.mp3",KeyPressed);
                ma_sound Sound = SoundFiles[KeyPressed - 'a'];
                if (!Sound.pDataSource)
                {
                    ma_sound_init_from_file(&engine, buffer, MA_SOUND_FLAG_DECODE, 0, 0, &Sound);
                    SoundFiles[KeyPressed - 'a'] = Sound;
                }
                if (Sound.pDataSource)
                {
                    ma_sound_seek_to_pcm_frame(&Sound, 0);
                    ma_sound_start(&Sound);
                }
            }
            snprintf(buffer,30,"%c",LastKeyCharPressed);
            PushDrawText(Renderer, Assets, font_type_times,  0.8f, 0.95f, buffer);

            RenderScene(Renderer, Assets);

            EndRenderPass();
        }

        WaitForFrameTimer(&FrameTimer, ExpectedMillisecondsPerFrame);
    }

    ma_engine_uninit(&engine);
    EndFrameTimer();
    CloseHandle(Assets->PlatformHandle);
    CloseVulkan();
}
