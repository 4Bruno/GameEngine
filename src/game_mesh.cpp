#include "game.h"
#include <inttypes.h>

struct obj_file_header
{
    uint32 VertexStart;    
    uint32 VertexCount;

    uint32 TextureCoordStart;
    uint32 TextureCoordCount;

    uint32 VertexNormalStart;
    uint32 VertexNormalCount;

    uint32 ParameterSpaceVerticesStart;
    uint32 ParameterSpaceVerticesCount;

    uint32 FaceElementsStart;
    uint32 FaceElementsCount;

    uint32 LineElementsStart;
    uint32 LineElementsCount;
};

/* ------------------------- BEGIN MODEL LOADER EMBEDDED ------------------------- */
void
SkipLine(const char * Data, uint32 Size,uint32 * ci)
{
    uint32 c = *ci;
    for (; (c < Size && Data[c] != '\n') ;++c)
    {
    };
    *ci = ++c;
}


inline void
AdvanceAfterWs(const char * Data, uint32 Size, uint32 * c)
{
    uint32 ci = *c;
    for (; (ci < Size);++ci)
    {
        if ( (Data[ci] == ' ') || (Data[ci] == '\n') )
        {
            break;
        }
    }
    *c = ++ci;
}

obj_file_header
ReadObjFileHeader(const char * Data, uint32 Size)
{
    obj_file_header Description = {};

    uint32 TotalPolys = 0;
    uint32 ci = 0;


    // skip comment section
    for (; (ci < Size);)
    {
        if (Data[ci] == '#') 
        {
            SkipLine(Data,Size,&ci);
        }
        else
        {
            break;
        }

    }

    for (; (ci < Size);)
    {
        uint32 * Count = 0;
        uint32 * Start = 0;
        uint32 LineCount = 0;
        char ContinueIfA = 0;
        char ContinueIfB = 0;
        switch (Data[ci])
        {
            case 'v':
            {
                if ( ((ci + 1) < Size) && (Data[ci+1] == 'p') )
                {
                    Start = &Description.ParameterSpaceVerticesStart;
                    Count = &Description.ParameterSpaceVerticesCount;
                    ContinueIfA = 'v';
                    ContinueIfB = 'p';
                }
                else if ( ((ci + 1) < Size) && (Data[ci+1] == 't') )
                {
                    Start = &Description.TextureCoordStart;
                    Count = &Description.TextureCoordCount;
                    ContinueIfA = 'v';
                    ContinueIfB = 't';
                }
                else if ( ((ci + 1) < Size) && (Data[ci+1] == 'n') )
                {
                    Start = &Description.VertexNormalStart;
                    Count = &Description.VertexNormalCount;
                    ContinueIfA = 'v';
                    ContinueIfB = 'n';
                }
                else
                {
                    Start = &Description.VertexStart;
                    Count = &Description.VertexCount;
                    ContinueIfA = 'v';
                    ContinueIfB = ' ';
                }
            } break;
            case 'f':
            {
                Start = &Description.FaceElementsStart;
                Count = &Description.FaceElementsCount;
                ContinueIfA = 'f';
                ContinueIfB = ' ';
            } break;
            case 'l':
            {
                Start = &Description.LineElementsStart;
                Count = &Description.LineElementsCount;
                ContinueIfA = 'l';
                ContinueIfB = ' ';
            } break;
            case 'm':
            {
                ContinueIfA = 'm';
                ContinueIfB = ' ';
            } break;
            case 'o':
            {
                ContinueIfA = 'o';
                ContinueIfB = ' ';
            } break;
            case 's':
            {
                ContinueIfA = 's';
                ContinueIfB = ' ';
            } break;
            case 'u':
            {
                ContinueIfA = 'u';
                ContinueIfB = ' ';
            } break;
            default:
            {
                // Error incorrect obj file format
                Assert(0);
            } break;
        }

        if (Start)
        {
            *Start = ci;
        }

        SkipLine(Data,Size,&ci);
        ++LineCount;

        for (;((ci + 1) < Size);)
        {
            if ( (Data[ci] == ContinueIfA) && (Data[ci+1] == ContinueIfB) )
            {
                SkipLine(Data,Size,&ci);
                ++LineCount;
            }
            else
            {
                if (Count)
                {
                    *Count = LineCount;
                }
                break;
            }
        }
        // last case exit
        if (Count)
        {
            *Count = LineCount;
        }
    }

#if 0
    Log("File has %u vertices. Beginning at line %u",Description.VertexCount,Description.VertexStart);
    Log("File has %u vertices normal. Beginning at line %u",Description.VertexNormalCount,Description.VertexNormalStart);
    Log("File has %u faces. Beginning at line %u",Description.FaceElementsCount,Description.FaceElementsStart);
#endif


    return Description;
}

mesh
CreateMeshFromObjHeader(memory_arena * Arena,void * BufferVertices,obj_file_header Header, const char * Data, uint32 Size)
//CreateMeshFromObjHeader(memory_arena * Arena,obj_file_header Header, const char * Data, uint32 Size)
{
    mesh Mesh = {};

    uint32 FaceVertices = 3;

    uint32 Vertices = Header.FaceElementsCount * FaceVertices; 
    uint32 VerticesSize = Vertices * sizeof(vertex_point);

    Mesh.Vertices = (vertex_point *)BufferVertices;
    Mesh.VertexSize = VerticesSize;

    // create 2 temp arrays for vertices/normals we will shrink stack later
    uint32 SizeUniqueVertices = Header.VertexCount * sizeof(v3);
    v3 * UniqueVertexArray = (v3 *)PushSize(Arena,SizeUniqueVertices);

    // Pre-load list of unique vertices
    uint32 Line = 0;

    for (uint32 ci = (Header.VertexStart); 
            (Line < Header.VertexCount); 
            ++Line)
    {
        ci += 2; // fixed
        for (uint32 CoordinateIndex = 0;
                CoordinateIndex < FaceVertices;
                ++CoordinateIndex)
        {
            UniqueVertexArray[Line]._V[CoordinateIndex] = (real32)atof(Data + ci);
            AdvanceAfterWs(Data,Size,&ci);
        }
    }
    
    uint32 SizeUniqueNormals = Header.VertexNormalCount * sizeof(v3);
    v3 * UniqueNormalArray = (v3 *)PushSize(Arena,SizeUniqueNormals);

    Line = 0;

    for (uint32 ci = (Header.VertexNormalStart); 
            (Line < Header.VertexNormalCount); 
            ++Line)
    {
        ci += 3; // fixed
        for (uint32 CoordinateIndex = 0;
                CoordinateIndex < FaceVertices;
                ++CoordinateIndex)
        {
            UniqueNormalArray[Line]._V[CoordinateIndex] = (real32)atof(Data + ci);
            AdvanceAfterWs(Data,Size,&ci);
        }
    }

    // Create all vertices indices data
    char * End;
    Line = 0;
    uint32 Indice = 0;
    uint32 Base10 = 10;
    for (uint32 ci = (Header.FaceElementsStart); 
            (Line < Header.FaceElementsCount); 
            ++Line)
    {
        ci += 2; // fixed
        for (uint32 VertexIndex = 0;
                VertexIndex < FaceVertices;
                ++VertexIndex)
        {
            uint16 IndexVertexP = (uint16)strtoimax(Data + ci,&End, Base10) - (uint16)1;
            uint16 IndexVertexT = (uint16)strtoimax(End + 1,&End, Base10) - (uint16)1;
            uint16 IndexVertexN = (uint16)strtoimax(End + 1,&End, Base10) - (uint16)1;
            // P
            Mesh.Vertices[Indice].P = UniqueVertexArray[IndexVertexP];
            // TextCoord
            // TODO
            // N
            Mesh.Vertices[Indice].N = UniqueNormalArray[IndexVertexN];

            AdvanceAfterWs(Data,Size,&ci);
            ++Indice;
        }
    }

    // reduce arena stack
    Arena->CurrentSize -= (SizeUniqueVertices + SizeUniqueNormals);


#if 0
    for (uint32 i = 0; i < Vertices;++i)
    {
        Log("Pos: %f %f %f Normal: %f %f %f\n",Mesh.Vertices[i].P.x,Mesh.Vertices[i].P.y,Mesh.Vertices[i].P.z,Mesh.Vertices[i].N.x,Mesh.Vertices[i].N.y,Mesh.Vertices[i].N.z);
    }
#endif

    return Mesh;
}

/* ------------------------- END MODEL LOADER EMBEDDED ------------------------- */

void
PushMeshSize(memory_arena * Arena, uint32 DataSize, uint32 InstanceCount)
{
    uint32 TotalSize = (DataSize * InstanceCount);
    uint32 Align = RenderGetVertexMemAlign() - 1;
    TotalSize = (TotalSize + Align) &  ~Align;

    Assert((Arena->CurrentSize + TotalSize) < Arena->MaxSize);

    Arena->CurrentSize += TotalSize;
}

THREAD_WORK_HANDLER(LoadMesh)
{
    async_load_mesh * WorkData = (async_load_mesh *)Data;
    Assert(WorkData->ThreadArena);

    game_state * GameState = WorkData->GameState;
    mesh * Mesh = WorkData->Mesh;
    Assert(Mesh);

    memory_arena * Arena = &WorkData->ThreadArena->Arena;
    Assert(!Mesh->Loaded);

    int32 Result = -1;
    file_contents GetFileResult = GetFileContents(WorkData->Memory, Arena,WorkData->Path);

    if (GetFileResult.Success)
    {
        obj_file_header Header = 
            ReadObjFileHeader((const char *)GetFileResult.Base, GetFileResult.Size);
        mesh TempMesh   = 
            CreateMeshFromObjHeader(Arena,
                    WorkData->BufferVertices,
                    Header, 
                    (const char *)GetFileResult.Base, GetFileResult.Size);

        Mesh->Vertices       = TempMesh.Vertices; // vertex_point * Vertices;
        Mesh->VertexSize     = TempMesh.VertexSize; // uint32   VertexSize;
        Mesh->Indices        = TempMesh.Indices; // Typedef * Indices;
        Mesh->IndicesSize    = TempMesh.IndicesSize; // uint32   IndicesSize;

        // TODO GPU barrier sync
        RenderPushVertexData(Mesh->Vertices,Mesh->VertexSize, WorkData->BaseOffset);
        //RenderPushVertexData(Mesh->Vertices,Mesh->VertexSize, 1);
        //RenderPushIndexData(&GameState->IndicesArena, Mesh->Indices,Mesh->IndicesSize, 1);

        COMPILER_DONOT_REORDER_BARRIER;
        Mesh->Loaded = true;
    }

#if 0
    v3 dir[3] = {
        {1,0,0},{0,1,0},{0,0,1}
    };

    for (uint32 VectorDirIndex = 0;
            VectorDirIndex < 3;
            ++VectorDirIndex)
    {
        int imin, imax;
        ExtremePointsAlongDirection(dir[VectorDirIndex], Mesh->Vertices, Mesh->VertexSize / sizeof(vertex_point), &imin, &imax);
        v3 MinP = Mesh->Vertices[imin].P;
        v3 MaxP = Mesh->Vertices[imax].P;
    }
#endif

    ThreadEndArena(WorkData->ThreadArena);
}

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

mesh *
GetMesh(game_memory * Memory, game_state * GameState,uint32 ID)
{

    mesh * Mesh = (GameState->Meshes + ID);

    if (!Mesh->Loaded && !Mesh->LoadInProcess)
    {
        const char * Paths[2] = {
            "assets\\cube.obj",
            "assets\\human_male_triangles.obj"
        };
        const uint32 MeshSizes[2] = {
            36 * sizeof(vertex_point),
            4200 * sizeof(vertex_point),
        };
        thread_memory_arena * ThreadArena = GetThreadArena(GameState);
        if (ThreadArena)
        {
            Mesh->LoadInProcess = true;

            memory_arena * Arena = ThreadBeginArena(ThreadArena);

            async_load_mesh * Data = PushStruct(Arena,async_load_mesh);
            Data->Memory    = Memory;                  // game_memory * Memory;
            Data->GameState = GameState;               // game_state * GameState;
            uint32 LenPath  = StrLen(Paths[ID]) + 1;
            Data->Path      = (char *)PushSize(Arena,LenPath); // Char_S * Path;
            CopyStr(Data->Path, Paths[ID], LenPath);
            Data->ThreadArena = ThreadArena;
            Data->Mesh = Mesh;

            uint32 MeshSize = MeshSizes[ID];
            // For multi thread
            // Get current base for arena
            Data->BaseOffset = GameState->MeshesArena.CurrentSize;
            Data->BufferVertices = GameState->MeshesArena.Base + GameState->MeshesArena.CurrentSize;
            // Increment MeshArena with the known total size required
            PushMeshSize(&GameState->MeshesArena, MeshSize, 1);

            // TODO I don't like this
            // Multi thread mesh load, push arena size based on size
            // of the current requested mesh
            Mesh->OffsetVertices = Data->BaseOffset;
            PushSize(&GameState->VertexArena, MeshSize);
            

            //Mesh->OffsetIndices = GameState->IndicesArena.CurrentSize;
            
            Memory->AddWorkToWorkQueue(Memory->RenderWorkQueue , LoadMesh,Data);
        }
    }

    return Mesh;
}
