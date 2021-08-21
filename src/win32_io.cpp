#include <windows.h>
#include "game_platform.h"

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

void
Win32CloseFile(platform_open_file_result Result)
{
    if (Result.Handle)
    {
        CloseHandle(Result.Handle);
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

b32
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

