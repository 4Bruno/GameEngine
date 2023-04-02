#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <windows.h>
#include "game_platform.h"
#include "game_memory.h"
#include "game_math.h"

enum asset_tag
{
    asset_tag_rocky,

    asset_tag_male,
    asset_tag_adult,

    asset_tag_tree,

    asset_tag_quad,
    asset_tag_sphere,
    asset_tag_cube,

    asset_tag_noperspective,
    asset_tag_texturesampling,

    asset_tag_font,
    asset_tag_char,

    asset_tag_LOD,

    asset_tag_count
};


enum game_asset_type
{
    game_asset_type_texture_ground,

    game_asset_type_mesh_humanoid,
    game_asset_type_mesh_vegetation,
    game_asset_type_mesh_shape,

    game_asset_type_shader_vertex,
    game_asset_type_shader_fragment,

    game_asset_type_font,

    game_asset_type_sound,

    game_asset_type_count
};

enum asset_file_type
{
    asset_file_type_unknown,

    /* BEGIN types that need post-processing on GPU side */
    asset_file_type_mesh,
    asset_file_type_shader,
    asset_file_type_shader_vertex,
    asset_file_type_shader_fragment,
    asset_file_type_shader_geometry,
    asset_file_type_texture,
    asset_file_type_mesh_material,
    asset_file_type_font,

    asset_file_type_requires_gpu_post_processing,
    /* END types that need post-processing on GPU side */

    asset_file_type_sound,


    asset_file_type_count
};

#define FONT_BEGIN_CHAR ' '
#define FONT_END_CHAR '~'
#define FONT_COUNT  (FONT_END_CHAR - FONT_BEGIN_CHAR)

enum font_type
{
    font_type_unknown,
    font_type_times,

    font_type_count
};

#pragma pack(push,1)

#define MAGIC_NUMBER(a, b, c, d) \
    (((u32)(a) << 0) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))

struct bin_asset_file_header
{
#define MAGIC_NUMBER_VALUE MAGIC_NUMBER('B','A','F','H')
    u32 MagicNumber;
#define BIN_ASSETS_VERSION 1
    u32 Version;

    u32 TagCount;
    u32 AssetsTypeCount;
    u32 AssetsCount;

    u32 Tags;
    u32 AssetTypes;
    u32 Assets;
};

struct bin_shader
{
    i32 GPUShaderID;
};

struct bin_text
{
    u32 Width;
    u32 Height;
    u32 Channels;
    i32 LOD;
};
typedef bin_text bin_font;

struct bin_mesh
{
    u32 SizeVertices;
    u32 SizeIndices;
    /*
     * DATA LAYOUT:
     * vertex_point[0..n]
     * i32[0..n]
     */
};
struct bin_sound
{
    // TODO
};

struct font_char_info
{
    r32 OffsetY;
};

struct bin_asset
{
    u32 ID;

    game_asset_type AssetType;
    asset_file_type FileType;

    u32 TagBegin;
    u32 TagOnePastLast;

    u32 DataBeginOffset;
    u32 Size;

    union
    {
        bin_text    Text;
        bin_mesh    Mesh;
        bin_sound   Sound;
        bin_font    Font;
        bin_shader  Shader;
    };
};

struct bin_asset_type
{
    game_asset_type ID;

    u32 Begin;
    u32 End;
};

struct bin_tag
{
    asset_tag ID;
    r32       Value;
};


#pragma pack(pop)

struct font_info
{
    font_type         Type;
    u32               LOD;
    r32               PixelHeight;
    r32               Ascent;
    font_char_info    Chars[FONT_COUNT];
    u8                * Bitmap;
    u32               Size;
    u32               Width;
    u32               Height;
};

struct asset_source
{
    char * Name;

    asset_file_type FileType;
    game_asset_type AssetType;

    u32 TagBegin;
    u32 TagOnePastLast;
};

/* Temp placeholder with large size */ 
#define LARGE_NUMBER 4096
struct bin_game_assets
{
    u32 AssetsCount;
    u32 TagsCount;

    bin_asset_type AssetTypes[game_asset_type_count];
    bin_tag Tags[LARGE_NUMBER];

    asset_source AssetsSource[LARGE_NUMBER];
    bin_asset AssetsBin[LARGE_NUMBER];


    bin_asset_type * DebugBinAssetType;
};

struct bucket
{
    i16 Coord[3];
    u32 Index;
};

struct mesh_header
{
    u32 SizeVertices;
    u32 SizeIndices;
};

struct vertex_point 
{
    v3 P;
    v3 N;
    v4 Color;
    v2 UV;
};


struct mesh_result
{
    b32 Success;
    vertex_point * Vertices;
    i16 * Indices;
    mesh_header Header;
};


#endif
