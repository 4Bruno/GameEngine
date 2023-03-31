#include "preprocessor_assets.h"

#include "win32_io.cpp"
#include <inttypes.h>

#pragma warning(disable:4244)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma warning(default:4244)

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

/*extern*/        b32  GlobalAppRunning     = false;
/*extern*/        b32 AllowMouseConfinement = false;
/*extern*/        on_window_resize * GraphicsOnWindowResize = 0;

struct file_read_result
{
    void * Data;
    DWORD Size;
    b32 Success;
};

internal b32
ReadFromFile(HANDLE Handle, void * Buffer, u32 Size)
{
    DWORD BytesRead = 0;
    ReadFile(Handle, Buffer, Size, &BytesRead, NULL);

    Assert(BytesRead == Size);

    return (BytesRead == Size);
}

internal file_read_result
Win32OpenReadFile(const char * Filename)
{
    file_read_result Result = {};

    HANDLE blob = 
        CreateFile(Filename, 
                    GENERIC_READ, 
                    FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,
                    OPEN_EXISTING, 
                    FILE_ATTRIBUTE_NORMAL, NULL);      

    if (blob == INVALID_HANDLE_VALUE)
    {
        return Result;
    }
    
    DWORD FileSizeHigh = 0;

    DWORD FileSizeLow = GetFileSize(blob, &FileSizeHigh);

    if (FileSizeLow == INVALID_FILE_SIZE)
    {
        CloseHandle(blob);
        return Result;
    }

    void * data = malloc(FileSizeLow);

    Result.Data = data;
    Result.Size = FileSizeLow;
    Result.Success = ReadFromFile(blob, data, FileSizeLow);

    CloseHandle(blob);

    return Result;
}

b32
WriteToFile(HANDLE Handle, void * Data, u32 Size, u32 Offset)
{
    DWORD BytesWritten = 0;
    DWORD Result = SetFilePointer(Handle, Offset, NULL,FILE_BEGIN);
    Assert(Result == Offset);
    WriteFile(Handle,Data, Size, &BytesWritten, NULL); 
    Assert(BytesWritten == Size);

    return (BytesWritten == Size);
}

b32
AppendToFile(HANDLE Handle, void * Data, u32 Size)
{
    DWORD BytesWritten = 0;
    WriteFile(Handle,Data, Size, &BytesWritten, NULL); 
    Assert(BytesWritten == Size);

    return (BytesWritten == Size);
}

void
SetOccupancy(hash_table * Map, bucket ** Bucket)
{
    u64 Index = ((size_t)*Bucket - (size_t)Map->Bucket) / sizeof(bucket);
    u64 OccupancyIndex = Index / 64;
    u64 OccupancyOffset = Index % 64; 
    u64 * Bit = (Map->Occupancy + OccupancyIndex);
    *Bit = *Bit | ((u64)0x01 << OccupancyOffset);
}

b32
IsOccupied(hash_table * Map, u32 Index)
{
    u32 OccupancyIndex = Index / 64;
    u32 OccupancyOffset = Index % 64; 
    u64 BitIsSet = *(Map->Occupancy + OccupancyIndex) & ((u64)0x01 << OccupancyOffset);

    return (BitIsSet > 0);
}



b32
GetBucketOrNext(hash_table * Map, i16 face[3], bucket ** BucketPtr)
{
    u32 Hash = (face[0] * 0x84028b + face[1] * 0x4ca91 + face[2] * 0x87631a);
#if 0
    Hash = Hash * Hash;
    Hash &= 0x0FF0;
    Hash *= Hash;
#endif

    u32 Index =  Hash & (Map->BucketCount - 1);
    u32 StartIndex = Index;
    b32 Found = 0;
    
    bucket * Bucket = Map->Bucket + Index;
    Map->CountLookups += 1;
    Map->CostFinding += 1;

    while ( IsOccupied(Map, Index) )
    {
        if ( (
                    Bucket->Coord[0] == face[0] &&
                    Bucket->Coord[1] == face[1] &&
                    Bucket->Coord[2] == face[2]
              ))
        {
            Found = 1;
            break;
        }
        else
        {
            Index = ++Index & (Map->BucketCount - 1);
        }
        if (Index == StartIndex)
        {
            Assert(0); // we loop entire table. Every bucket is occuppeid and not found
        }

        Map->CostFinding += 1;
        Bucket = Map->Bucket + Index;
    };

    *BucketPtr = Bucket;

    return Found;
}


void
strcpy(char * dest, char * src)
{
    size_t i = 0;
    for (; 
         i < strlen(src);
         i++)
    {
        if (src[i] == 0) break;
        dest[i] = src[i];
    }
    dest[i] = 0;
}

asset_file_type
GetAssetFileTypeFromName(const char * Filename)
{
    u32 len = (u32)strlen(Filename) - 1;

    char FileExt[12];
    u32 CountChars = 0;
    for (u32 i = len; 
            (i > 0) || (CountChars < ArrayCount(FileExt)); 
            i--)
    {
        char c = Filename[i];
        if (c == '.') break;
        FileExt[CountChars++] = c;
    }

    Assert(CountChars < ArrayCount(FileExt));

    for (u32 i = 0; i < (CountChars / 2);++i)
    {
        char c = FileExt[i];
        int swap = CountChars - i - 1;
        FileExt[i] = FileExt[swap];
        FileExt[swap] = c;
    }
    FileExt[CountChars] = 0;

    asset_file_type FileType = asset_file_type_unknown;

    if      (strcmp(FileExt,"obj") == 0)
    {
        FileType = asset_file_type_mesh;
    }
    else if (strcmp(FileExt, "jpg")  == 0)
    {
        FileType = asset_file_type_texture;
    }
    else if (strcmp(FileExt, "png")  == 0)
    {
        FileType = asset_file_type_texture;
    }
    else if (strcmp(FileExt, "mtl")  == 0)
    {
        FileType = asset_file_type_mesh_material;
    }
    else if (strcmp(FileExt, "spv")  == 0)
    {
        FileType = asset_file_type_shader;
    }
    else if (strcmp(FileExt, "vert")  == 0)
    {
        FileType = asset_file_type_unknown;
    }
    else if (strcmp(FileExt, "frag")  == 0)
    {
        FileType = asset_file_type_unknown;
    }
    else if (strcmp(FileExt, "h")  == 0)
    {
        FileType = asset_file_type_unknown;
    }
    else if (strcmp(FileExt, "ttf")  == 0)
    {
        FileType = asset_file_type_font;
    }
    else if (strcmp(FileExt, "mp3")  == 0)
    {
        FileType = asset_file_type_sound;
    }
    else
    {
        Logn("No handler for %s", FileExt);
        Assert(0);
    }

    return FileType;
}

#if 0
b32
GetFileFullPath(const char * FindFileWithName, char * Buffer)
{
    b32 Found = false;

    const char * path[2] = 
    {
        ".\\assets",
        ".\\shaders"
    };

    WIN32_FIND_DATA ffd;

    for (u32 i = 0;
             i < ArrayCount(path);
             ++i)
    {
        const char * Path = path[i];
        char PathWildChar[MAX_PATH];
        strcpy_s(PathWildChar, Path);
        size_t PathLen = strlen(Path);
        PathWildChar[PathLen+0] = '\\';
        PathWildChar[PathLen+1] = '*';
        PathWildChar[PathLen+2] = 0;

        HANDLE hFind = FindFirstFile(PathWildChar, &ffd);

        if (INVALID_HANDLE_VALUE == hFind) 
        {
            Logn("Invalid dir %s",Path);
        } 

        do
        {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                  strcmp(ffd.cFileName, FindFileWithName) == 0)
            {
                strcpy(Buffer, path[i] );
                Buffer[strlen(path[i])] = '\\';
                strcpy(Buffer + strlen(path[i]), ffd.cFileName);
                Found = true;
                break;
            }
        }
        while (FindNextFile(hFind, &ffd) != 0);

        FindClose(hFind);
    }

    return Found;
}
#endif


void
SkipLine(const char * Data, u32 Size,u32 * ci)
{
    u32 c = *ci;
    for (; (c < Size && Data[c] != '\n') ;++c)
    {
    };
    *ci = ++c;
}

int
ReadWord(const char * Data, u32 Size, u32 &ci, char * buffer, u32 BufferSize)
{
    u32 i = 0;
    for (; (ci < Size && Data[ci] != '\n') ;++ci)
    {
        if (Data[ci] == ' ')
        {
            break;
        }
        else
        {
            buffer[i] = Data[ci];
            ++i;
            if (i >= BufferSize)
            {
                Logn("ReadWord failed. BufferSize not big enough (%i).", BufferSize);
                return 1;
            }
        }
    };
    buffer[i] = 0;

    return 0;
}

inline void
AdvanceAfterWs(const char * Data, u32 Size, u32 * c)
{
    u32 ci = *c;
    for (; (ci < Size);++ci)
    {
        if ( (Data[ci] == ' ') || (Data[ci] == '\n') )
        {
            break;
        }
    }
    *c = ++ci;
}

b32
NewHashTable(hash_table * Map, u32 NumberOfBuckets)
{
    Map->BucketCount = NextPowerOf2(NumberOfBuckets);
    u32 SizeBucket = sizeof(bucket) * 3 * Map->BucketCount;
    Map->Bucket = (bucket *)malloc(SizeBucket);
    RtlZeroMemory(Map->Bucket, SizeBucket);
    u32 SizeOccupancy = ((Map->BucketCount / 64) + 1) * sizeof(u64);
    Map->Occupancy = (u64*)malloc(SizeOccupancy);
    RtlZeroMemory(Map->Occupancy, SizeOccupancy);
    Map->CostFinding = 0;
    Map->CountLookups = 0;

    return 0;
}

void
FreeHashTable(hash_table * Map)
{
    if (Map)
    {
        free(Map->Occupancy);
        free(Map->Bucket);
    }
}

mesh_result
PreprocessMesh(const char * Data, u32 Size)
{
    mesh_result Result = {};
    Result.Success = false;

    u32 ci = 0;
    for (; ci < Size; ++ci)
    {
        if (Data[ci] == 'o' && Data[ci+1] == ' ') break; 
    } 

    SkipLine(Data, Size, &ci);

    u32 VerticesCount = 0;

    u32 TrianglesCount = 0;

    u32 VerticesNormalCount = 0;

    u32 TextureUVCount = 0;

    u32 DummyLineCounter = 0;
    u32 * LineCounter = 0;

    char LineType[10];
    if (ReadWord(Data, Size, ci, LineType, sizeof(LineType)))
    {
        Logn("Error processing .obj file");
        return Result;
    }

    for (; ci < Size;)
    {
        char LineTypeNext[10];

        if      (strcmp("v",LineType) == 0)
        {
            LineCounter = &VerticesCount;
        }
        else if (strcmp("vt",LineType) == 0)
        {
            LineCounter = &TextureUVCount;
        }
        else if (strcmp("vn",LineType) == 0)
        {
            LineCounter = &VerticesNormalCount;
        }
        else if (strcmp("f",LineType) == 0)
        {
            LineCounter = &TrianglesCount;
        }
        else
        {
            LineCounter = &DummyLineCounter;
        }

        *LineCounter += 1;

        do
        {

            SkipLine(Data, Size, &ci);
            if (ReadWord(Data, Size, ci, LineTypeNext, sizeof(LineTypeNext)))
            {
                Logn("Error processing .obj file");
                return Result;
            }
            if (strcmp(LineTypeNext,LineType) != 0)
            {
                strcpy(LineType,LineTypeNext);
                break;
            }
            else
            {
                (*LineCounter) += 1;
            }

        } while (ci < Size);
    } // pre process header 

    v3 * Vertices = (v3 *)malloc(sizeof(v3) * VerticesCount);
    v2 * UV = (v2 *)malloc(sizeof(v2) * TextureUVCount);
    v3 * Normals = (v3 *)malloc(sizeof(v3) * VerticesNormalCount);
    u32 TotalVertices = TrianglesCount * 3;

    i16 * Faces = (i16 *)malloc(sizeof(i16) * TotalVertices);
    // worst case size
    vertex_point * VertexPoints = (vertex_point *)malloc(sizeof(vertex_point) * TotalVertices);
    u32 UniqueVertexPoints = 0;

    VerticesCount = 0;
    TrianglesCount = 0;
    VerticesNormalCount = 0;
    TextureUVCount = 0;

    hash_table Map;
    if (NewHashTable(&Map, TotalVertices))
    {
        Logn("Error creating hash table");
        return Result;
    }

    ci = 0;
    for (; ci < Size;)
    {

        if (ReadWord(Data, Size, ci, LineType, sizeof(LineType)))
        {
            Logn("Error processing .obj file");
            return Result;
        }

        u32 start_c = ci + 1;

        SkipLine(Data, Size, &ci);

        if      (strcmp("v",LineType) == 0)
        {
            v3 * v = Vertices + VerticesCount++;
            v->x = (r32)atof(Data + start_c);
            AdvanceAfterWs(Data, Size, &start_c);
            v->y = (r32)atof(Data + start_c);
            AdvanceAfterWs(Data, Size, &start_c);
            v->z = (r32)atof(Data + start_c);
        }
        else if (strcmp("vt",LineType) == 0)
        {
            v2 * uv = UV + TextureUVCount++;
            uv->x = 1.0f * (r32)atof(Data + start_c);
            AdvanceAfterWs(Data, Size, &start_c);
            uv->y = (1.0f - (r32)atof(Data + start_c));
        }
        else if (strcmp("vn",LineType) == 0)
        {
            v3 * v = Normals + VerticesNormalCount++;
            v->x = (r32)atof(Data + start_c);
            AdvanceAfterWs(Data, Size, &start_c);
            v->y = (r32)atof(Data + start_c);
            AdvanceAfterWs(Data, Size, &start_c);
            v->z = (r32)atof(Data + start_c);
        }
        else if (strcmp("f",LineType) == 0)
        {
            char * End_c = 0;
            u32 Base10 = 10;

            for (int i = 0; i < 3; ++i)
            {
                i16 Indices[3];
                Indices[0] = (i16)strtoimax((const char *)(Data + start_c),&End_c, Base10) - (i16)1;
                Indices[1] = (i16)strtoimax((const char *)(End_c + 1),&End_c, Base10) - (i16)1;
                Indices[2] = (i16)strtoimax((const char *)(End_c + 1),&End_c, Base10) - (i16)1;

                bucket * Bucket = 0;
                b32 Found = GetBucketOrNext(&Map, Indices, &Bucket);

                if (Found)
                {
                    //Logn("Vertex duplicated already exists (%i %i %i)", Indices[0], Indices[1], Indices[2]);
                }
                else
                {
                    Bucket->Coord[0] = Indices[0];
                    Bucket->Coord[1] = Indices[1];
                    Bucket->Coord[2] = Indices[2];
                    Bucket->Index = UniqueVertexPoints++;
                    SetOccupancy(&Map,&Bucket); 

                    // TODO: Create vertex_point array
                    vertex_point * Vertex = VertexPoints + Bucket->Index;
                    Vertex->P = Vertices[Indices[0]];
                    Vertex->UV = UV[Indices[1]];
                    Vertex->N = Normals[Indices[2]];
                }

                Assert(Bucket->Index < (1 << 16));
                // Point to the index of vertex in array
                Faces[TrianglesCount++] = (i16)Bucket->Index;
                AdvanceAfterWs(Data, Size, &start_c);
            }
        }

    } // pre process header 
    
    //Logn("Unique number of vertices %i", UniqueVertexPoints);

    //Logn("Ratio hash map lookup %i over %i = %i", Map.CountLookups, Map.CostFinding, (Map.CostFinding / Map.CountLookups));
    FreeHashTable(&Map);
    free(Vertices);
    free(UV);
    free(Normals);

    // those are buffer of unique vertices and triangle indexes
    //free(Faces);
    //free(VertexPoints);
    Result.Success = true;
    Result.Vertices = VertexPoints;
    Result.Header.SizeVertices = sizeof(vertex_point) * UniqueVertexPoints;
    Result.Indices = Faces;
    Result.Header.SizeIndices = sizeof(i16) * TotalVertices;

    return Result;
}


void 
TestHashTable()
{
    hash_table Map;
    NewHashTable(&Map,16);

    i16 Faces[3][3] = {
        { 1, 3, 5},
        { 2, 4, 6},
        { 1, 3, 5}
    };

    bucket * Bucket = 0;
    for (int i = 0; i < 3; ++i)
    {
        b32 Found = GetBucketOrNext(&Map, Faces[i], &Bucket);
        if (Found)
        {
            Logn("Found!");
        }
        else
        {
            Bucket->Coord[0] = Faces[i][0];
            Bucket->Coord[1] = Faces[i][1];
            Bucket->Coord[2] = Faces[i][2];
            SetOccupancy(&Map,&Bucket); 
        }
    }

    FreeHashTable(&Map);
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
        //i32 H = (iy1 - iy0);
        *MaxW = (W > *MaxW) ? W : *MaxW;
        *MaxH = (W > *MaxH) ? W : *MaxH;
    }
}

font_info
CreateFontTexture(char * TTFBuffer, font_type FontType, u32 LOD)
{
    r32 LODToPixelHeight[2] = {
        16.0f,
        32.0f
    };

    Assert(LOD <= ArrayCount(LODToPixelHeight));
    r32 PixelHeight = LODToPixelHeight[LOD];

    font_info FontInfo = {};

    stbtt_fontinfo Font;
    stbtt_InitFont(&Font, (u8 *)TTFBuffer, stbtt_GetFontOffsetForIndex((u8 *)TTFBuffer,0));

    r32 TextHeight = PixelHeight;
    i32 TotalWidth = 0;
    i32 TotalHeight = 0;

    r32 ScaleY = stbtt_ScaleForPixelHeight(&Font, TextHeight);
    r32 ScaleX = ScaleY;
    //i32 Width ,Height ,XOffset ,YOffset;

    i32 MaxWidth = 0;
    i32 MaxHeight = 0;


    i32 CountChars = 0;
    i32 CharsRanges[][2] = {
        {' ','~'},
    };

    for (i32 i = 0; i < (i32)ArrayCount(CharsRanges); ++i)
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
    for (i32 i = 0; i < (i32)ArrayCount(CharsRanges); ++i)
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
            FontInfo.Chars[c - FONT_BEGIN_CHAR].OffsetY = (r32)iy0 / (r32)H;
            i32 PaddingX = (MaxWidth - W) / 2;
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

    i32 Ascent, Descent, Linegap;
    stbtt_GetFontVMetrics(&Font, &Ascent, &Descent, &Linegap);
    FontInfo.Ascent = (Ascent * ScaleY) / MaxWidth;
    FontInfo.PixelHeight = (r32)MaxHeight;
    FontInfo.Type = FontType;
    FontInfo.LOD = LOD;
    FontInfo.Bitmap = AtlastBuffer;
    FontInfo.Width = TotalWidth;
    FontInfo.Height = TotalHeight;

    return FontInfo;
}


void
BeginAssetType(bin_game_assets * Assets, game_asset_type Type)
{
    // check begin is always followed by end before starting new pack
    Assert(Assets->DebugBinAssetType == 0);
    Assets->DebugBinAssetType = Assets->AssetTypes + Type;

    // has not been initialized. All struct must be ordered
    Assert( (Assets->DebugBinAssetType->Begin == 0) &&
            (Assets->DebugBinAssetType->End   == 0) );

    Assets->DebugBinAssetType->ID     = Type;
    Assets->DebugBinAssetType->Begin  = Assets->AssetsCount;
    Assets->DebugBinAssetType->End    = Assets->AssetsCount;
}
void
EndAssetType(bin_game_assets * Assets, game_asset_type Type)
{
    Assert(Assets->DebugBinAssetType);
    Assert(Assets->DebugBinAssetType->ID == Type);

    bin_asset_type * AssetType = Assets->AssetTypes + Type;

    AssetType->End    = Assets->AssetsCount;

    Assert(AssetType->End >= AssetType->Begin);

    Assets->DebugBinAssetType = 0;
}

void
AddAsset(bin_game_assets * Assets, const char * Filename)
{
    Assert(Assets->DebugBinAssetType);
    asset_source * Asset = Assets->AssetsSource + Assets->AssetsCount++;

    Asset->TagBegin       = Assets->TagsCount;
    Asset->TagOnePastLast = Assets->TagsCount;
    Asset->AssetType      = Assets->DebugBinAssetType->ID;

    Asset->FileType =  
        GetAssetFileTypeFromName(Filename);

    Asset->Name = (char *)Filename;

    //GetFileFullPath(Filename, Asset->Name);
}

asset_source *
GetCurrentAsset(bin_game_assets * Assets)
{
    Assert(Assets->AssetsCount > 0);
    return (Assets->AssetsSource + (Assets->AssetsCount - 1));
}
void
AddTag(bin_game_assets * Assets, asset_tag Tag, r32 Value)
{
    asset_source * Asset = GetCurrentAsset(Assets);
    bin_tag * TagSlot = Assets->Tags + Assets->TagsCount;

    TagSlot->ID = Tag;
    TagSlot->Value = Value;

    Asset->TagOnePastLast = ++Assets->TagsCount;
}


void
WriteBinaryAssetsFile(bin_game_assets * Assets)
{
   bin_asset_file_header BAF; 

   BAF.MagicNumber  = MAGIC_NUMBER_VALUE;
   BAF.Version      = BIN_ASSETS_VERSION;

   BAF.AssetsCount      = Assets->AssetsCount;
   BAF.TagCount         = Assets->TagsCount;
   BAF.AssetsTypeCount  = game_asset_type_count;

   // begin addr offset
   u32 ByteSizeTags       = BAF.TagCount        * sizeof(bin_tag);
   u32 ByteSizeAssetType  = BAF.AssetsTypeCount * sizeof(bin_asset_type);
   u32 ByteSizeAssets     = BAF.AssetsCount     * sizeof(bin_asset);

   BAF.Tags       = sizeof(bin_asset_file_header);
   BAF.AssetTypes = BAF.Tags + ByteSizeTags;
   BAF.Assets     = BAF.AssetTypes + ByteSizeAssetType;

   HANDLE blob = 
       CreateFile("assets.bin", 
               GENERIC_WRITE | GENERIC_READ, 
               FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,
               CREATE_ALWAYS, 
               FILE_ATTRIBUTE_NORMAL, NULL);      


   // Reserve space
   AppendToFile(blob,&BAF               , sizeof(bin_asset_file_header)); 
   AppendToFile(blob,Assets->Tags       , ByteSizeTags); 
   AppendToFile(blob,Assets->AssetTypes , ByteSizeAssetType); 
   AppendToFile(blob,Assets->AssetsBin  , ByteSizeAssets); 

   // Allocate/pre-process files
   u32 DataBeginOffset = BAF.Assets + ByteSizeAssets;

   u32 MaxDataSize = 0;
   u32 MinDataSize = Megabytes(10);
   u32 AssetIndexBegin = 1; // ignore first bucket 
                            
   Logn("|--------------------------------------------------|--MB--|");
   for (u32 AssetIndex = AssetIndexBegin;
           AssetIndex < Assets->AssetsCount;
           ++AssetIndex)
   {
       asset_source * AssetSource = Assets->AssetsSource + AssetIndex;
       bin_asset *AssetBin = Assets->AssetsBin + AssetIndex;

       file_read_result File = Win32OpenReadFile(AssetSource->Name);

       //Logn("Processing AssetIndex (%i)", AssetIndex);

       u32 DataSize         = 0;

       if (File.Success)
       {
            switch (AssetSource->FileType)
            {
                case asset_file_type_mesh:
                {
                    mesh_result Mesh = PreprocessMesh((const char *)File.Data, File.Size);
                    Assert(Mesh.Success);

                    AppendToFile(blob, Mesh.Vertices  , Mesh.Header.SizeVertices);
                    AppendToFile(blob, Mesh.Indices   , Mesh.Header.SizeIndices);

                    free(Mesh.Indices);
                    free(Mesh.Vertices);

                    AssetBin->Mesh.SizeVertices       = Mesh.Header.SizeVertices;
                    AssetBin->Mesh.SizeIndices        = Mesh.Header.SizeIndices;
                    AssetBin->Mesh.GPUIndecesOffset   = UINT32_MAX;
                    AssetBin->Mesh.GPUVerticesOffset  = UINT32_MAX;

                    DataSize = Mesh.Header.SizeIndices + 
                               Mesh.Header.SizeVertices;

                } break;

                case asset_file_type_texture:
                {
                    int x,y,n;
                    int desired_channels = 4;

                    stbi_uc * Image = stbi_load_from_memory((const unsigned char *)File.Data, File.Size, &x, &y, &n, desired_channels);
                    Assert(Image);

                    u32 ImageSize = x * y * desired_channels;
                    AppendToFile(blob,Image, ImageSize);
                    stbi_image_free(Image);

                    AssetBin->Text.Height         = y;
                    AssetBin->Text.Width          = x;
                    AssetBin->Text.Channels       = desired_channels;
                    AssetBin->Text.GPUTextureID   = -1;

                    DataSize = ImageSize;

                } break;

                case asset_file_type_shader:
                {
                    AppendToFile(blob,File.Data, File.Size);

                    AssetBin->Shader.GPUShaderID = -1;

                    DataSize = File.Size;
                } break;

                case asset_file_type_font:
                {
                    bin_font * Font = &AssetBin->Font;
                    font_type FontType = font_type_unknown;
                    for (u32 i = AssetSource->TagBegin;
                             i < AssetSource->TagOnePastLast;
                            ++i)
                    {
                        bin_tag * Tag = Assets->Tags + i;
                        if (Tag->ID == asset_tag_font) 
                        { 
                            FontType = (font_type)Tag->Value; 
                        }
                        else if (Tag->ID == asset_tag_LOD)
                        {
                            Font->LOD = (i32)Tag->Value;
                        }
                    } 

                    Assert( (FontType > font_type_unknown) &&
                            (FontType < font_type_count));
                    Assert(Font->LOD >= 0);

                    font_info TimesFont = 
                        CreateFontTexture((char *)File.Data,FontType, Font->LOD);

                    AppendToFile(blob,TimesFont.Chars, ArrayCount(TimesFont.Chars) * sizeof(font_char_info));
                    AppendToFile(blob,TimesFont.Bitmap, TimesFont.Width * TimesFont.Height);
                    free(TimesFont.Bitmap);

                    bin_font * BitmapText = &AssetBin->Font;
                    BitmapText->Height        = TimesFont.Height;
                    BitmapText->Width         = TimesFont.Width;
                    BitmapText->Channels      = 1;
                    BitmapText->GPUTextureID  = -1;

                    DataSize = ArrayCount(TimesFont.Chars) * sizeof(font_char_info) +
                               (TimesFont.Width * TimesFont.Height);
                } break;
                case asset_file_type_sound:
                {
                    AppendToFile(blob,File.Data, File.Size);

                    DataSize = File.Size;
                } break;

                case asset_file_type_unknown: break;
                case asset_file_type_shader_vertex: break;
                case asset_file_type_shader_fragment: break;
                case asset_file_type_shader_geometry: break;
                case asset_file_type_mesh_material: break;
            };
       }

       free(File.Data);
       Assert(DataSize > 0);
       Logn("|%-50s|%-5f|", AssetSource->Name,((r32)DataSize / 1024.0f ));
       if (DataSize > MaxDataSize) MaxDataSize = DataSize;
       if (DataSize < MinDataSize) MinDataSize = DataSize;
       AssetBin->Size             = DataSize;
       AssetBin->DataBeginOffset  = DataBeginOffset;
       AssetBin->AssetType        = AssetSource->AssetType;
       AssetBin->FileType         = AssetSource->FileType;
       AssetBin->TagBegin         = AssetSource->TagBegin;
       AssetBin->TagOnePastLast   = AssetSource->TagOnePastLast;

       DataBeginOffset += DataSize;
   }

   Logn("Uncompressed MAX data file in KB %f", ((r32)MaxDataSize / 1024.0f ));
   Logn("Uncompressed MIN data file in KB %f", ((r32)MinDataSize / 1024.0f ));

   // Commit updates on binary assets array
   WriteToFile(blob, Assets->Tags         , ByteSizeTags      , BAF.Tags);
   WriteToFile(blob, Assets->AssetTypes   , ByteSizeAssetType , BAF.AssetTypes);
   WriteToFile(blob, Assets->AssetsBin    , ByteSizeAssets    , BAF.Assets);
}

void
CreateAssetsMeta(bin_game_assets * Assets)
{
   BeginAssetType(Assets, game_asset_type_texture_ground);
   AddAsset(Assets, "assets\\ground_stone_01.jpg");
   AddTag(Assets, asset_tag_rocky,1.0f);

   AddAsset(Assets, "assets\\ground_stone_02.jpg");
   AddTag(Assets, asset_tag_rocky,0.1f);
   EndAssetType(Assets, game_asset_type_texture_ground);

   BeginAssetType(Assets, game_asset_type_mesh_humanoid);
   AddAsset(Assets, "assets\\human_male_triangles.obj");
   AddTag(Assets, asset_tag_male, 1.0f);
   AddTag(Assets, asset_tag_adult, 0.4f);
   EndAssetType(Assets, game_asset_type_mesh_humanoid);

   BeginAssetType(Assets, game_asset_type_mesh_vegetation);
   AddAsset(Assets, "assets\\palm_tree.obj");
   AddTag(Assets, asset_tag_tree, 1.0f);
   EndAssetType(Assets, game_asset_type_mesh_vegetation);

   BeginAssetType(Assets, game_asset_type_mesh_shape);
   AddAsset(Assets, "assets\\cube_triangles.obj");
   AddTag(Assets, asset_tag_cube, 1.0f);

   AddAsset(Assets, "assets\\sphere.obj");
   AddTag(Assets, asset_tag_sphere, 1.0f);

   AddAsset(Assets, "assets\\quad.obj");
   AddTag(Assets, asset_tag_quad, 1.0f);
   EndAssetType(Assets, game_asset_type_mesh_shape);


   BeginAssetType(Assets, game_asset_type_shader_vertex);
   AddAsset(Assets, "shaders\\test.vert.spv");
   AddTag(Assets, asset_tag_noperspective, 1.0f);
   EndAssetType(Assets, game_asset_type_shader_vertex);

   BeginAssetType(Assets, game_asset_type_shader_fragment);
   AddAsset(Assets, "shaders\\triangle_text.frag.spv");
   AddTag(Assets, asset_tag_texturesampling, 1.0f);
   EndAssetType(Assets, game_asset_type_shader_fragment);

   BeginAssetType(Assets, game_asset_type_sound);
   AddAsset(Assets, "assets\\letter_a.mp3");
   AddTag(Assets, asset_tag_char, 'a');
   EndAssetType(Assets, game_asset_type_sound);

    const char * FontsToLoad[] = {
        "c:/windows/fonts/times.ttf"
    };
    font_type FontsToLoadType[] = {
        font_type_times
    };

   BeginAssetType(Assets, game_asset_type_font);
   for (u32 LOD = 0;
           LOD < 1;
           ++LOD)
   {
       for (u32 FontIndex = 0;
               FontIndex < ArrayCount(FontsToLoad);
               ++FontIndex)
       {
           char * Filename = (char *)FontsToLoad[FontIndex];
           AddAsset(Assets,Filename);
           AddTag(Assets,asset_tag_font, (r32)FontsToLoadType[FontIndex]);
           AddTag(Assets,asset_tag_LOD, (r32)LOD);
       }
   }
   EndAssetType(Assets, game_asset_type_font);

   WriteBinaryAssetsFile(Assets);
}


void
TestAssets(bin_game_assets * Assets)
{
#pragma warning(disable:4127)
    Assert(ArrayCount(CTAssetTypeNames) == game_asset_type_count);
    Assert(ArrayCount(CTTagNames)       == asset_tag_count);
#pragma warning(default:4127)

    Logn("%s","--------------------------------------------------");
    for (u32 i = 0;
             i < game_asset_type_count;
             ++i)
    {
        bin_asset_type * AssetType = Assets->AssetTypes + i;
        Logn("%-50s:%i", CTAssetTypeNames[i],(AssetType->End - AssetType->Begin));    
    }
    Logn("%s","--------------------------------------------------");

    for (u32 i = 0;
             i < Assets->AssetsCount;
             ++i)
    {
        asset_source * Asset = Assets->AssetsSource + i;
        u32 TagCount = Asset->TagOnePastLast - Asset->TagBegin;
        Logn("Asset %s(%i) (Tags: %i)", Asset->Name, i, TagCount);
        for (u32 TagI = Asset->TagBegin;
                 TagI < Asset->TagOnePastLast;
                 ++ TagI)
        {
            bin_tag * Tag = Assets->Tags + TagI;
            Logn("Tag %i with k:%s v:%f", TagI, CTTagNames[Tag->ID], Tag->Value);
        }
        Logn("%s","--------------------------------------------------");
    }
}



void
InitializeAssets(bin_game_assets * Assets)
{
    // initialize to 1 to have index 0 as invalid asset
    Assets->AssetsCount = 1;
    Assets->TagsCount   = 1;
    Assets->Tags[0].ID = (asset_tag)0;
    Assets->Tags[0].Value= 10000.0f;
    Assets->AssetsBin[0].AssetType = (game_asset_type)0;
    Assets->AssetsBin[0].DataBeginOffset = 0;
}

#if 1
int
main()
{
    bin_game_assets Assets_;
    bin_game_assets * Assets = &Assets_;
    InitializeAssets(Assets);
    CreateAssetsMeta(Assets);
    TestAssets(Assets);
    return 0;
}
#else
int
main()
{
    const char * path[2] = 
    {
        ".\\assets",
        ".\\shaders"
    };

    HANDLE blob = 
        CreateFile("assets.bin", 
                    GENERIC_WRITE | GENERIC_READ, 
                    FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,
                    CREATE_ALWAYS, 
                    FILE_ATTRIBUTE_NORMAL, NULL);      

    asset_header Headers[200];

    asset_file_header AFH = CreateHeaders(path, ArrayCount(path), Headers, ArrayCount(Headers));

    u32 SizeAssetsHeaders = sizeof(asset_header) * ArrayCount(Headers);

    u32 FileBufferOffset = sizeof(asset_file_header) + 
                           SizeAssetsHeaders;

    for (u32 HeaderIndex = 0;
             HeaderIndex < AFH.CountHeaders;
             ++HeaderIndex)
    {
        asset_header * header = Headers + HeaderIndex;
        Logn("File: %-50s FileType:%i Size: %i", header->Filename, header->FileType, header->Size);

        file_read_result ReadResult = Win32OpenReadFile(header->Filename);

        if (ReadResult.Success)
        {

            switch (header->FileType)
            {
                case asset_file_type_mesh:
                    {
                        mesh_result Mesh = PreprocessMesh((const char *)ReadResult.Data, header->Size);
                        if (Mesh.Success)
                        {
                            AppendToFile(blob, &Mesh.Header, sizeof(mesh_header));
                            AppendToFile(blob, Mesh.Vertices, Mesh.Header.SizeVertices);
                            AppendToFile(blob, Mesh.Indices, Mesh.Header.SizeIndices);

                            free(Mesh.Indices);
                            free(Mesh.Vertices);

                            header->DataBeginOffset = FileBufferOffset;
                            FileBufferOffset += sizeof(mesh_header) + 
                                Mesh.Header.SizeIndices + 
                                Mesh.Header.SizeVertices;
                        }
                    } break;
                case asset_file_type_texture:
                {
                    AppendToFile(blob,ReadResult.Data, ReadResult.Size);
                    header->DataBeginOffset = FileBufferOffset;
                    FileBufferOffset += ReadResult.Size;
                } break;
                case asset_file_type_shader:
                {
                    AppendToFile(blob,ReadResult.Data, ReadResult.Size);
                    header->DataBeginOffset = FileBufferOffset;
                    FileBufferOffset += ReadResult.Size;
                } break;

                case asset_file_type_unknown: break;
                case asset_file_type_sound: break;
                case asset_file_type_shader_vertex: break;
                case asset_file_type_shader_fragment: break;
                case asset_file_type_shader_geometry: break;
                case asset_file_type_mesh_material: break;
            };

            int NameLen = strlen(header->Filename);
            int Backslash;
            for (Backslash = (NameLen - 1);
                             Backslash >= 0;
                             --Backslash)
            {
                if (header->Filename[Backslash] == '\\')
                {
                    break;
                }
            }
            if (Backslash > 0)
            {
                strcpy(header->Filename,header->Filename + Backslash + 1);
            }
        
            free(ReadResult.Data);
        }
        else
        {
            Logn("Unable to read file %s", header->Filename);
        }
    }

    char * FontsToLoad[] = {
        "c:/windows/fonts/times.ttf"
    };
    font_type FontsToLoadType[] = {
        font_type_times
    };

    for (u32 LOD = 0;
             LOD < 1;
             ++LOD)
    {
        for (u32 FontIndex = 0;
                FontIndex < ArrayCount(FontsToLoad);
                ++FontIndex)
        {
            char * Filename = FontsToLoad[FontIndex];
            font_type FontType = FontsToLoadType[FontIndex];
            font_info TimesFont = 
                CreateFontTexture(Filename,FontType, LOD);
            AppendToFile(blob,TimesFont.Bitmap, TimesFont.Width * TimesFont.Height);
            free(TimesFont.Bitmap);
        }
    }

    if (AFH.CountHeaders)
    {
        WriteToFile(blob, Headers, SizeAssetsHeaders, 0);
    }

    CloseHandle(blob);

    return 0;   
}
#endif
