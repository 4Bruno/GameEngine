#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <windows.h>
#include "game_platform.h"
#include "game_memory.h"
#include "game_math.h"
#include "game_assets.h"

enum asset_file_type
{
    asset_file_type_unknown,
    asset_file_type_mesh,
    asset_file_type_sound,
    asset_file_type_shader,
    asset_file_type_shader_vertex,
    asset_file_type_shader_fragment,
    asset_file_type_shader_geometry,
    asset_file_type_texture,
    asset_file_type_mesh_material
};

struct file_header
{
    u32 CountHeaders;
};

struct asset_data
{
    u32 Size;
    void * Begin;
};

struct asset_header
{
    u32 Size;                     // 4
    asset_file_type FileType;     // 4  8
    u32 LengthName;               // 4  12
    u32 DataBeginOffset;          // 4  16
    char Filename[52];            // 52 68
};

struct assets_handler
{
    u32 CountAssets;
    asset_header * Headers;

    void * PlatformHandle;
    memory_arena Arena;
};


struct bucket
{
    i16 Coord[3];
    u32 Index;
};

struct hash_table
{
    bucket * Bucket;
    u64 * Occupancy;
    u32 BucketCount;

    u32 CountLookups;
    u32 CostFinding;
};

struct mesh_header
{
    u32 SizeVertices;
    u32 SizeIndices;
};

struct mesh_result
{
    b32 Success;
    vertex_point * Vertices;
    i16 * Indices;
    mesh_header Header;
};


inline void
InitializeArena(memory_arena * Arena,u8 * BaseAddr, u32 MaxSize)
{
    Arena->MaxSize = MaxSize;
    Arena->CurrentSize = 0;
    Arena->Base = BaseAddr;
}
inline u32
Win32RewindFile(HANDLE handle, u32 Offset = 0)
{
    DWORD Result = SetFilePointer(handle, Offset, NULL,FILE_BEGIN);
    return Result;
}


#endif
