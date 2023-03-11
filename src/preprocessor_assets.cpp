#include "preprocessor_assets.h"

#include "win32_io.cpp"
#include "game_math.h"
#include "game_assets.h"
#include <inttypes.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#define PLATFORM_OPEN_HANDLE(name) platform_open_file_result name(const char * Filepath)
typedef PLATFORM_OPEN_HANDLE(platform_open_handle);

#define PLATFORM_CLOSE_HANDLE(name) void name(platform_open_file_result OpenFileResult)
typedef PLATFORM_CLOSE_HANDLE(platform_close_handle);

#define PLATFORM_READ_HANDLE(name) b32 name(platform_open_file_result OpenFileResult, void * Buffer)
typedef PLATFORM_READ_HANDLE(platform_read_handle);


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

struct asset_header
{
    u32 Size;                     // 4
    asset_file_type FileType;     // 4  8
    u32 LengthName;               // 4  12
    u32 DataBeginOffset;          // 4  16
    char Filename[52];            // 52 68
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

void
SetOccupancy(hash_table * Map, bucket ** Bucket)
{
    u64 Index = ((size_t)*Bucket - (size_t)Map->Bucket) / sizeof(bucket);
    u32 OccupancyIndex = Index / 64;
    u32 OccupancyOffset = Index % 64; 
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

u32 
NextPowerOf2(u32 N)
{
    if (!(N & (N - 1)))
        return N;
    // else set only the left bit of most significant bit
    //return 0x8000000000000000 >> (__builtin_clzll(N) - 1); // gnu
    //return 0x80000000 >> (__lzcnt(N) - 1); // 32 bit
    return 0x8000000000000000 >> (__lzcnt64(N) - 1);
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

u32
Win32RewindFile(HANDLE handle, u32 Offset = 0)
{
    DWORD Result = SetFilePointer(handle, Offset, NULL,FILE_BEGIN);
    return Result;
}

asset_header
CreateHeaderFromFile(const char * dir, WIN32_FIND_DATA ffd, LARGE_INTEGER filesize)
{
    asset_header header = {};

    filesize.LowPart = ffd.nFileSizeLow;
    filesize.HighPart = ffd.nFileSizeHigh;
    //Logn("  %s   %llu bytes", ffd.cFileName, filesize.QuadPart);
    u32 len = (u32)strlen(ffd.cFileName) - 1;
    char FileExt[12];
    int CountChars = 0;
    for (u32 i = len; 
            (i > 0) || (CountChars < ArrayCount(FileExt)); 
            i--)
    {
        char c = ffd.cFileName[i];
        if (c == '.') break;
        FileExt[CountChars++] = c;
    }

    Assert(CountChars < ArrayCount(FileExt));

    for (int i = 0; i < (CountChars / 2);++i)
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

    if (FileType != asset_file_type_unknown)
    {
        Assert(len < ArrayCount(header.Filename));

        header.LengthName = len;
        header.DataBeginOffset = 0;
        header.Size = (u32)filesize.LowPart;

        strcpy(header.Filename, dir);
        int DirLen = strlen(dir);
        if (header.Filename[DirLen -1] != '\\')
        {
            header.Filename[DirLen] = '\\';
            DirLen += 1;
        }
        strcpy(header.Filename + DirLen, ffd.cFileName);

    }

    header.FileType = FileType;

    return header;

}

int
CreateHeaders(const char ** dir,int NumberOfDirs, HANDLE blob)
{
    WIN32_FIND_DATA ffd;
    LARGE_INTEGER filesize;

    file_header FileHeader = {};
    DWORD BytesWritten = 0;

    WriteFile(blob, &FileHeader, sizeof(file_header), &BytesWritten, NULL);

    for (int i = 0;
             i < NumberOfDirs;
             ++i)
    {
        const char * Path = dir[i];
        char PathWildChar[MAX_PATH];
        strcpy(PathWildChar, Path);
        int PathLen = strlen(Path);
        PathWildChar[PathLen+0] = '\\';
        PathWildChar[PathLen+1] = '*';
        PathWildChar[PathLen+2] = 0;

        HANDLE hFind = FindFirstFile(PathWildChar, &ffd);

        if (INVALID_HANDLE_VALUE == hFind) 
        {
            Logn("Invalid dir %s",dir);
        } 

        do
        {
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                //Logn("Directory %s", ffd.cFileName);
            }
            else
            {
                asset_header AssetHeader = CreateHeaderFromFile( Path, ffd, filesize);
                if (AssetHeader.FileType != asset_file_type_unknown)
                {
                    WriteFile(blob, &AssetHeader, sizeof(asset_header), &BytesWritten, NULL);
                    Assert(sizeof(asset_header) == BytesWritten);

                    FileHeader.CountHeaders += 1;
                }
            }
        }
        while (FindNextFile(hFind, &ffd) != 0);

        FindClose(hFind);
    }

    if (Win32RewindFile(blob) == INVALID_SET_FILE_POINTER)
    {
        Logn("Failed to set file pointer");
        return 1;
    }

    WriteFile(blob, &FileHeader, sizeof(file_header), &BytesWritten, NULL);

    FlushFileBuffers(blob);

    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES) 
    {
        Logn("Error during reading directory files %lu", dwError);
    }

    return 0;
}


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

    i32 VerticesBegin = 0;
    u32 VerticesCount = 0;

    i32 TrianglesBegin = 0;
    u32 TrianglesCount = 0;

    i32 VerticesNormalBegin = 0;
    u32 VerticesNormalCount = 0;

    i32 TextureUVBegin = 0;
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
        int start_c = ci;

        if      (strcmp("v",LineType) == 0)
        {
            VerticesBegin = start_c;
            LineCounter = &VerticesCount;
        }
        else if (strcmp("vt",LineType) == 0)
        {
            TextureUVBegin = start_c;
            LineCounter = &TextureUVCount;
        }
        else if (strcmp("vn",LineType) == 0)
        {
            VerticesNormalBegin = start_c;
            LineCounter = &VerticesNormalCount;
        }
        else if (strcmp("f",LineType) == 0)
        {
            TrianglesBegin = start_c;
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
            uv->x = (r32)atof(Data + start_c);
            AdvanceAfterWs(Data, Size, &start_c);
            uv->y = (r32)atof(Data + start_c);
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

                // Point to the index of vertex in array
                Faces[TrianglesCount++] = Bucket->Index;
                AdvanceAfterWs(Data, Size, &start_c);
            }
        }

    } // pre process header 
    
    //Logn("Unique number of vertices %i", UniqueVertexPoints);

    Logn("Ratio hash map lookup %i over %i = %i", Map.CountLookups, Map.CostFinding, (Map.CostFinding / Map.CountLookups));
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

struct file_read_result
{
    void * Data;
    DWORD Size;
    b32 Success;
};

file_read_result
Win32ReadFile(const char * Filename)
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

    DWORD BytesRead = 0;
    BOOL bResult = ReadFile(blob, data, FileSizeLow, &BytesRead, NULL);

    if (BytesRead != FileSizeLow)
    {
        CloseHandle(blob);
        return Result;
    }


    CloseHandle(blob);

    Result.Data = data;
    Result.Size = FileSizeLow;
    Result.Success = true;

    return Result;
}

void
TestReadFileAfterUpdate()
{
    HANDLE blob = 
        CreateFile("assets.bin", 
                    GENERIC_READ, 
                    FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,
                    OPEN_EXISTING, 
                    FILE_ATTRIBUTE_NORMAL, NULL);      

    file_header FileHeader;

    DWORD BytesRead = 0;
    DWORD BytesWritten = 0;

    BOOL bResult = ReadFile(blob, &FileHeader, sizeof(file_header), &BytesRead, NULL);
    Assert(BytesRead == sizeof(file_header));
    Logn("Total header %i",FileHeader.CountHeaders); 

    u32 SizeAssetsHeaders = sizeof(asset_header) * FileHeader.CountHeaders;
    asset_header * Headers = (asset_header *)malloc(SizeAssetsHeaders);
    bResult = ReadFile(blob, Headers, SizeAssetsHeaders, &BytesRead, NULL);
    Assert(BytesRead == SizeAssetsHeaders);

    u32 FileBufferOffset = sizeof(file_header) + 
                           SizeAssetsHeaders;

    for (u32 i = 0;
             i < FileHeader.CountHeaders;
             ++i)
    {
        asset_header * header = Headers + i;
        Logn("File: %-50s FileType:%i Size: %i", header->Filename, header->FileType, header->Size);

        Win32RewindFile(blob, header->DataBeginOffset);

        switch (header->FileType)
        {
            case asset_file_type_mesh:
            {
                mesh_header MeshHeader;
                ReadFile(blob, &MeshHeader, sizeof(mesh_header), &BytesRead, NULL);

                vertex_point * Vertices = (vertex_point *)malloc(MeshHeader.SizeVertices);
                ReadFile(blob, Vertices, MeshHeader.SizeVertices, &BytesRead, NULL);
                Assert(MeshHeader.SizeVertices == BytesRead);

                u32 CountVertices = MeshHeader.SizeVertices / sizeof(vertex_point);
#if 0
                for (int i = 0; i < CountVertices;++i)
                {
                    vertex_point * vertex = Vertices + i;
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
                bResult = ReadFile(blob, Raw, header->Size, &BytesRead, NULL);
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
        };
    }

    free(Headers);
    CloseHandle(blob);
}


int
main()
{
#if 0
    TestHashTable();
    return 0;
#endif
#if 0
    TestReadFileAfterUpdate();
    return 0;
#endif
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

    CreateHeaders(path, ArrayCount(path), blob);

    file_header FileHeader;

    DWORD BytesRead = 0;
    DWORD BytesWritten = 0;

    if (Win32RewindFile(blob) == INVALID_SET_FILE_POINTER)
    {
        Logn("Failed to set file pointer");
        return 1;
    }
    BOOL bResult = ReadFile(blob, &FileHeader, sizeof(file_header), &BytesRead, NULL);
    Assert(BytesRead == sizeof(file_header));
    Logn("Total header %i",FileHeader.CountHeaders); 

    u32 SizeAssetsHeaders = sizeof(asset_header) * FileHeader.CountHeaders;
    asset_header * Headers = (asset_header *)malloc(SizeAssetsHeaders);
    bResult = ReadFile(blob, Headers, SizeAssetsHeaders, &BytesRead, NULL);
    Assert(BytesRead == SizeAssetsHeaders);

    u32 FileBufferOffset = sizeof(file_header) + 
                           SizeAssetsHeaders;

    for (u32 i = 0;
             i < FileHeader.CountHeaders;
             ++i)
    {
        asset_header * header = Headers + i;
        Logn("File: %-50s FileType:%i Size: %i", header->Filename, header->FileType, header->Size);

        file_read_result ReadResult = Win32ReadFile(header->Filename);

        if (ReadResult.Success)
        {

            switch (header->FileType)
            {
                case asset_file_type_mesh:
                    {
                        mesh_result Mesh = PreprocessMesh((const char *)ReadResult.Data, header->Size);
                        if (Mesh.Success)
                        {
                            WriteFile(blob, &Mesh.Header, sizeof(mesh_header), &BytesWritten, NULL);

                            WriteFile(blob, Mesh.Vertices, Mesh.Header.SizeVertices, &BytesWritten, NULL);
                            Assert(Mesh.Header.SizeVertices == BytesWritten);
                            WriteFile(blob, Mesh.Indices, Mesh.Header.SizeIndices, &BytesWritten, NULL);
                            Assert(Mesh.Header.SizeIndices == BytesWritten);
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
                    WriteFile(blob,ReadResult.Data, ReadResult.Size, &BytesWritten, NULL); 
                    Assert(BytesWritten == ReadResult.Size);
                    header->DataBeginOffset = FileBufferOffset;
                    FileBufferOffset += ReadResult.Size;
                } break;
                case asset_file_type_shader:
                {
                    WriteFile(blob,ReadResult.Data, ReadResult.Size, &BytesWritten, NULL); 
                    Assert(BytesWritten == ReadResult.Size);
                    header->DataBeginOffset = FileBufferOffset;
                    FileBufferOffset += ReadResult.Size;
                } break;
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

    if (FileHeader.CountHeaders)
    {
        if (Win32RewindFile(blob, sizeof(file_header)) == INVALID_SET_FILE_POINTER)
        {
            Logn("Failed to set file pointer");
            return 1;
        }

        WriteFile(blob, Headers, SizeAssetsHeaders, &BytesWritten, NULL);
    }

    free(Headers);
    CloseHandle(blob);

    return 0;   
}
