#include "preprocessor_assets.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "vulkan\vulkan_win32.h"
#include "vulkan_initializer.cpp"
//#include "vulkan_initializer.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define APP_NAME "preprocessor_test"
#define APP_WINDOW_WIDTH  980
#define APP_WINDOW_HEIGHT 540 
#define ENABLED_DEBUG_VULKAN_VALIDATION_LAYER 1

// windowsx.h
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#define QUAD_TO_MS(Q) Q.QuadPart * (1.0f / 1000.0f)

static b32 GlobalAppRunning = true;

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

platform_open_file_result
Win32OpenFileReadOnly(const char * file)
{
    platform_open_file_result Result = {};

    HANDLE FileHandle = CreateFileA(file,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);

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


assets_handler
ProcessAssetsFile(memory_arena * Arena, u32 MaxAssetsArenaSize)
{
    assets_handler GameAssets = {};
    GameAssets.Arena.Base = Arena->Base + Arena->CurrentSize;
    GameAssets.Arena.MaxSize = MaxAssetsArenaSize;

    platform_open_file_result File = Win32OpenFileReadOnly("assets.bin");

    file_header FileHeader;

    DWORD BytesRead = 0;

    ReadFile(File.Handle, &FileHeader, sizeof(file_header), &BytesRead, NULL);
    Assert(BytesRead == sizeof(file_header));
    Logn("Total header %i",FileHeader.CountHeaders); 

    u32 SizeAssetsHeaders = sizeof(asset_header) * FileHeader.CountHeaders;
    asset_header * Headers = PushArray(Arena, FileHeader.CountHeaders, asset_header);
    PushArray(&GameAssets.Arena, FileHeader.CountHeaders, asset_header);

    ReadFile(File.Handle, Headers, SizeAssetsHeaders, &BytesRead, NULL);
    Assert(BytesRead == SizeAssetsHeaders);

    for (u32 i = 0;
             i < FileHeader.CountHeaders;
             ++i)
    {
        asset_header * header = Headers + i;
        Logn("File: %-50s FileType:%i Size: %i", header->Filename, header->FileType, header->Size);

        Win32RewindFile(File.Handle, header->DataBeginOffset);

        switch (header->FileType)
        {
            case asset_file_type_mesh:
            {
                mesh_header MeshHeader;
                ReadFile(File.Handle, &MeshHeader, sizeof(mesh_header), &BytesRead, NULL);

                vertex_point * Vertices = (vertex_point *)malloc(MeshHeader.SizeVertices);
                ReadFile(File.Handle, Vertices, MeshHeader.SizeVertices, &BytesRead, NULL);
                Assert(MeshHeader.SizeVertices == BytesRead);

                u32 CountVertices = MeshHeader.SizeVertices / sizeof(vertex_point);
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
                void * Raw = malloc(header->Size);
                ReadFile(File.Handle, Raw, header->Size, &BytesRead, NULL);
                Assert(header->Size == BytesRead);
                int x,y,n;
                int desired_channels = 4;
                stbi_uc * Image = stbi_load_from_memory((const unsigned char *)Raw, header->Size, &x, &y, &n, desired_channels);
                if (!Image)
                {
                    Logn("Error loading image %s",header->Filename);
                }
                else
                {
                    Logn("Image loaded with size %i %i %i (Offset:%i)", x,y,n, header->DataBeginOffset);
                }
                free(Raw);
            } break;

            case asset_file_type_unknown: break;
            case asset_file_type_sound: break;
            case asset_file_type_shader: 
            {
                void * Raw = malloc(header->Size);
                ReadFile(File.Handle, Raw, header->Size, &BytesRead, NULL);
                Assert(header->Size == BytesRead);
                i32 Shader = CreateShaderModule(Raw, header->Size);
                if (Shader < 0)
                {
                    Logn("Unable to load shader %s", header->Filename);
                }
                else
                {
                    Logn("Shader loaded succesful %s", header->Filename);
                }
                free(Raw);
            } break;
            case asset_file_type_shader_vertex: break;
            case asset_file_type_shader_fragment: break;
            case asset_file_type_shader_geometry: break;
            case asset_file_type_mesh_material: break;
        };
    }

    GameAssets.CountAssets = FileHeader.CountHeaders;
    GameAssets.Headers = Headers;
    GameAssets.PlatformHandle = File.Handle;

    return GameAssets;
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

i32
my_stbtt_initfont(void)
{

    DWORD BytesRead = 0;
    platform_open_file_result file_ttf = Win32OpenFileReadOnly("c:/windows/fonts/times.ttf");

    if (!file_ttf.Success) return -1;

    void * buffer = malloc(file_ttf.Size);

    ReadFile(file_ttf.Handle, buffer, file_ttf.Size, &BytesRead, NULL);
    Assert(BytesRead == file_ttf.Size);

    stbtt_fontinfo Font;
    stbtt_InitFont(&Font, (u8 *)buffer, stbtt_GetFontOffsetForIndex((u8 *)buffer,0));

    r32 ScaleY = 300.0f;
    i32 codepoint;
    i32 Width ,Height ,XOffset ,YOffset;
    i32 ix0, iy0, ix1, iy1;
    u8 * bitmap = stbtt_GetCodepointBitmap(&Font, 
                                           stbtt_ScaleForPixelHeight(&Font, ScaleY),stbtt_ScaleForPixelHeight(&Font, ScaleY), 
                                           'A', &Width , &Height , &XOffset,&YOffset);
    
    ttf_tex = PushTextureData(bitmap, Width, Height, 1);

#if 0
    glBindTexture(GL_TEXTURE_2D, ftex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
    // can free temp_bitmap at this point
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif


    stbtt_FreeBitmap(bitmap, 0);
    free(buffer);

    return ttf_tex;
}

void
DrawText(i32 Pipeline, i32 Texture,  const char * text, u32 OffsetVertex, u32 OffsetIndices, r32 TimeElapsed)
{
    WaitForRender();

    v4 ClearColor = V4(0,0,0,1);

    RenderBeginPass(ClearColor);

#if 0
    mesh_push_constant PushConstant = {};
    PushConstant.ImageIndex = Texture;
    RenderPushVertexConstant(sizeof(mesh_push_constant),&PushConstant);
#endif

    gpu_memory_mapping_result ObjectsMapResult = 
        BeginObjectMapping(1);

    if (!ObjectsMapResult.Success)
    {
        Assert(0);
        return;
    }

    GPUObjectData * ObjectData = (GPUObjectData *)ObjectsMapResult.BeginAddress;

    //m4 Scale = M4(V3(0.5f));
    ObjectData->ModelMatrix = M4();
    r32 Z = (sinf(TimeElapsed / 2.0f) * 5.0f);
    //Logn("Z: %f", Z);
    ObjectData->ModelMatrix[0][2] = Z;
    ObjectData->Color = V4(1.0f,1.0f,1.0f,0);
    ObjectData->ImageIndex = Texture;

    EndObjectsArena();

    RenderSetPipeline(Pipeline);
    //RenderBindTexture(Texture);
    //RenderDrawObjectNTimes(8,1,0);
    RenderPushMeshIndexed(1, 6, OffsetVertex, OffsetIndices);

#if 0
    while (*text) {
        if ((*text) >= 32 && (*text) < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, 512,512, *text-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
            
            glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0);
            glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
            glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1);
            glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1);
        }
        ++text;
    }
#endif

    EndRenderPass();
}

asset_data
GetAsset(assets_handler * Assets, const char * Name)
{
    asset_data AssetData = {};

    for (i32 i = 0; 
             i < Assets->CountAssets; 
             ++i)
    {
        asset_header * H = Assets->Headers + i;
        if (strcmp(H->Filename,Name) == 0)
        {
            DWORD BytesRead = 0;
            void * Buffer = (void *)PushSize(&Assets->Arena, H->Size);
            Win32RewindFile(Assets->PlatformHandle, H->DataBeginOffset);
            ReadFile(Assets->PlatformHandle,Buffer,H->Size, &BytesRead, NULL);
            Assert(BytesRead == H->Size);
            AssetData.Begin = Buffer;
            AssetData.Size = H->Size;
        }
    }

    return AssetData;
}

i32
CreateFontTexture()
{
    i32 TextIndex = -1;

    DWORD BytesRead = 0;
    platform_open_file_result file_ttf = Win32OpenFileReadOnly("c:/windows/fonts/times.ttf");

    if (!file_ttf.Success) return - 1;

    void * buffer = malloc(file_ttf.Size);

    ReadFile(file_ttf.Handle, buffer, file_ttf.Size, &BytesRead, NULL);
    Assert(BytesRead == file_ttf.Size);

    stbtt_fontinfo Font;
    stbtt_InitFont(&Font, (u8 *)buffer, stbtt_GetFontOffsetForIndex((u8 *)buffer,0));

    i32 LastLetter = 'A' + 10;
    r32 ScaleY = stbtt_ScaleForPixelHeight(&Font, 150.0f);
    i32 Width ,Height ,XOffset ,YOffset;

#if 0
    i32 TotalWidth = 0;
    i32 TotalHeight = 0;
    for (i32 c = 'A'; c <= LastLetter; ++c)
    {
        int ix0, iy0, ix1, iy1;
        stbtt_GetCodepointBitmapBox(&Font, c, ScaleY, ScaleY, &ix0, &iy0, &ix1, &iy1);
        TotalWidth += (ix1 - ix0);
        TotalHeight += (iy1 - iy0);
    }
    void * Text = malloc(TotalWidth * TotalHeight * 1);
    u8 * TextPtr = (u8 *) Text;
    for (i32 c = 'A'; c <= LastLetter; ++c)
    {
#if 1
        int ix0, iy0, ix1, iy1;
        stbtt_GetCodepointBitmapBox(&Font, c, ScaleY, ScaleY, &ix0, &iy0, &ix1, &iy1);
        i32 W = (ix1 - ix0);
        i32 H = (iy1 - iy0);
        //Logn("%c: %i %i",c, ix1 - ix0, iy1 - iy0);
        stbtt_MakeCodepointBitmapSubpixel(&Font, TextPtr, W, H, W, ScaleY, ScaleY,0.0f, 0.0f, c);
        TextPtr += (W * H * 1);
#else
        u8 * bitmap = stbtt_GetCodepointBitmap(&Font, ScaleY,ScaleY, c, &Width , &Height , &XOffset,&YOffset);
        Logn("%c: %i %i (%i %i)",c, Width, Height, XOffset, YOffset);
        stbtt_FreeBitmap(bitmap, 0);
#endif
    }

    TextIndex = PushTextureData(Text, TotalWidth, TotalHeight, 1);
#else
    for (i32 c = 'A'; c <= LastLetter; ++c)
    {
        u8 * bitmap = stbtt_GetCodepointBitmap(&Font, ScaleY,ScaleY, c , &Width , &Height , &XOffset,&YOffset);
        TextIndex = PushTextureData(bitmap, Width, Height, 1);
        stbtt_FreeBitmap(bitmap, 0);
    }
#endif
    free(buffer);

    return TextIndex;
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

    assets_handler Assets = ProcessAssetsFile(&Arena, Megabytes(10));
    asset_data VertexShader = GetAsset(&Assets,"test.vert.spv");
    asset_data FragmentShader = GetAsset(&Assets,"triangle_text.frag.spv");

    i32 VS = CreateShaderModule(VertexShader.Begin,VertexShader.Size);
    i32 FS = CreateShaderModule(FragmentShader.Begin,FragmentShader.Size);
    pipeline_creation_result Pipeline = CreatePipeline(VS, FS, polygon_mode_fill);

    asset_data SquareObj = GetAsset(&Assets,"quad.obj");
    mesh_header * Mesh = (mesh_header *)SquareObj.Begin;
    vertex_point * Vertices = (vertex_point *)((u8*)SquareObj.Begin + sizeof(mesh_header));
    vertex_point * Indices = (vertex_point *)((u8*)SquareObj.Begin + sizeof(mesh_header) + Mesh->SizeVertices);
    
    u32 GPUVertexOffset = 0, GPUIndicesOffset = 0;
    PushVertexData(Vertices, Mesh->SizeVertices, &GPUVertexOffset);
    RenderPushIndexData(Indices, Mesh->SizeIndices, &GPUIndicesOffset);

#if 1
    //i32 Texture = my_stbtt_initfont();
    i32 MaxTextures = CreateFontTexture();
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

    while (GlobalAppRunning)
    {
        BeginFrameTimer(&FrameTimer);

        win32_window_dim WinDim = Win32GetWindowSize(WindowHandle);

        Input.TimeElapsed = CalculateTimeElapsed(&FrameTimer);

        HandleInput(&Input.Controller,WindowHandle, WinDim.Width, WinDim.Height);

        if (!GlobalWindowIsMinimized)
        {
            i32 Texture = ((i32)Input.TimeElapsed % MaxTextures);
            //Texture = 1;
            DrawText(Pipeline.Pipeline, Texture, "THis is a test", GPUVertexOffset, GPUIndicesOffset, Input.TimeElapsed);
        }

        WaitForFrameTimer(&FrameTimer, ExpectedMillisecondsPerFrame);
    }

    EndFrameTimer();
    CloseHandle(Assets.PlatformHandle);
    CloseVulkan();
}
