#include <Windows.h>
#include "game_platform.h"
#include "game_memory.h"
#include "win32_io.cpp"
#include "model_loader.cpp"
#include "data_load.cpp"
#include <stdio.h>

inline uint32
StrLen(const char * c)
{
    uint32 ci = 0;
    for (;
            (c[ci] != 0);
            ++ci)
    { }
    return ci;
}

inline void
CopyStr(char * DestStr,const char * SrcStr ,uint32 Length)
{
    for (uint32 ci = 0;
                ci < Length;
                ++ci)
    {
        DestStr[ci] = SrcStr[ci];
    }
}

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


struct file_contents_ext
{
    file_contents Content;
    char * Path;
};

file_contents_ext * 
Win32GetAllFilesInDir(game_memory * Memory, memory_arena * Arena,const char * Dir, uint32 * outCountFiles)
{
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(Dir, &ffd);
    file_contents_ext * Files = 0;
    uint32 CountFiles = 0;

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                ++CountFiles;
            }
        }
        while (FindNextFile(hFind, &ffd) != 0);

        Files = PushArray(Arena,CountFiles,file_contents_ext);

        hFind = FindFirstFile(Dir, &ffd);

        CountFiles = 0;
        uint32 LengthDirName = StrLen(Dir); 
        char FileName[260];
        for (int32 ci = LengthDirName;
                    ci >= 0;
                    --ci)
        {
            if (Dir[ci] == '\\')
            {
                LengthDirName = ci;
                break;
            }
        }
        CopyStr(FileName, Dir, LengthDirName);
        CopyStr(FileName + LengthDirName, "\\", 1);
        LengthDirName += 1;
        do
        {
            if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                uint32 LengthFileName = StrLen(ffd.cFileName);
                CopyStr(FileName + LengthDirName, (const char *)ffd.cFileName, LengthFileName);
                FileName[LengthDirName + LengthFileName] = 0;
                file_contents FileContents = GetFileContents(Memory,Arena,FileName);

                if (FileContents.Success)
                {
                    Files[CountFiles].Content = FileContents;
                    Files[CountFiles].Path = (char *)PushSize(Arena, LengthDirName + LengthFileName + 1);
                    CopyStr(Files[CountFiles].Path,FileName,LengthDirName + LengthFileName + 1);
                    //Files[CountFiles].Path[LengthDirName + LengthFileName] = 0;
                }
            
                ++CountFiles;
            }
        }
        while (FindNextFile(hFind, &ffd) != 0);
        FindClose(hFind);
    }

    *outCountFiles = CountFiles;
    return Files;
}
DWORD
GetCwd(char * s, DWORD StrLength)
{
    DWORD cdSize= GetModuleFileNameA(0,s,StrLength);

    for (int32 ci = cdSize;
            ci >= 0;
            --ci)
    {
        if (s[ci] == '\\')
        {
            cdSize = ci;
            s[ci] = 0;
            break;
        }
    }

    return cdSize;
}

int main()
{
    HINSTANCE hInstance = GetModuleHandle(0);

    uint32 PermanentMemorySize = Megabytes(30);
    void * PermanentMemory = VirtualAlloc(0, PermanentMemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    memory_arena Arena = {};
    Arena.MaxSize = PermanentMemorySize;
    Arena.CurrentSize = 0;
    Arena.Base = (uint8 *)PermanentMemory;

    game_memory GameMemory     = {};
    GameMemory.DebugOpenFile       = Win32OpenFile;       // debug_open_file DebugOpenFile;
    GameMemory.DebugReadFile       = Win32ReadFile;       // debug_read_file DebugReadFile;
    GameMemory.DebugCloseFile      = Win32CloseFile;      // debug_close_file DebugCloseFile;

    char Dir[255];
    DWORD cdSize = GetCwd(Dir,255);
    char * TargetDir = "\\assets\\*.obj";
    CopyStr(Dir + cdSize, TargetDir, StrLen(TargetDir) + 1);
    
    uint32 CountFiles = 0;
    file_contents_ext * Files = Win32GetAllFilesInDir(&GameMemory, &Arena,Dir, &CountFiles);

    printf("Files found %i\n", (int32)CountFiles);

    uint32 FaceVertices = 3;
    for (uint32 FileIndex = 0;
                FileIndex < CountFiles;
                ++FileIndex)
    {
        const char * Buffer = (const char *)Files[FileIndex].Content.Base;
        uint32 FileSize = Files[FileIndex].Content.Size;
        obj_file_header Header = ReadObjFileHeader(Buffer, FileSize);
        printf("%s VertexCount: %i Vertices: %i\n",Files[FileIndex].Path, Header.VertexCount, Header.FaceElementsCount * FaceVertices);
    }

    return 0;
}
