#include "game.h"
#include <inttypes.h>
#include "static_mesh_models.cpp"

struct obj_file_header
{
    u32  CurrentByteIndex;
    char Name[100];

    u32 VertexStart;    
    u32 VertexCount;

    u32 TextureCoordStart;
    u32 TextureCoordCount;

    u32 VertexNormalStart;
    u32 VertexNormalCount;

    u32 ParameterSpaceVerticesStart;
    u32 ParameterSpaceVerticesCount;

    u32 FaceElementsStart;
    u32 FaceElementsCount;

    u32 LineElementsStart;
    u32 LineElementsCount;
};

/* ------------------------- BEGIN MODEL LOADER EMBEDDED ------------------------- */
void
SkipLine(const char * Data, u32 Size,u32 * ci)
{
    u32 c = *ci;
    for (; (c < Size && Data[c] != '\n') ;++c)
    {
    };
    *ci = ++c;
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

inline void
CopyStrUntilChar(char * DestStr,const char * SrcStr, char CharExit, u32 MaxLength)
{
    for (u32 ci = 0;
                ci < MaxLength;
                ++ci)
    {
        if (SrcStr[ci] == CharExit)
        {
            break;
        }
        else
        {
            DestStr[ci] = SrcStr[ci];
        }
    }
}

obj_file_header
ReadObjFileHeader(const char * Data, u32 StartAtByte, u32 Size)
{
    obj_file_header Description = {};

    u32 TotalPolys = 0;
    u32 ci = StartAtByte;


    // find first object
    for (; (ci < Size);)
    {
        if (Data[ci] != 'o') 
        {
            SkipLine(Data,Size,&ci);
        }
        else
        {
            CopyStrUntilChar(Description.Name,Data + ci + 2,'\n',sizeof(Description.Name));
            SkipLine(Data,Size,&ci);
            break;
        }

    }

    for (; (ci < Size);)
    {
        u32 * Count = 0;
        u32 * Start = 0;
        u32 LineCount = 0;
        char ContinueIfA = 0;
        char ContinueIfB = 0;

        // this will tell caller if multiple objects
        // where is the current index
        Description.CurrentByteIndex = ci;

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
                // Early exit dont parse next object
                // let caller handle it
                return Description;

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
    Logn("File has %u vertices. Beginning at line %u",Description.VertexCount,Description.VertexStart);
    Logn("File has %u vertices normal. Beginning at line %u",Description.VertexNormalCount,Description.VertexNormalStart);
    Logn("File has %u faces. Beginning at line %u",Description.FaceElementsCount,Description.FaceElementsStart);
#endif


    return Description;
}

struct parsed_obj_header
{
};

mesh
CreateMeshFromObjHeader(memory_arena * Arena,void * BufferVertices,obj_file_header Header, const char * Data, u32 Size,
                        u32 OffsetVertexP, u32 OffsetVertexN)
//CreateMeshFromObjHeader(memory_arena * Arena,obj_file_header Header, const char * Data, u32 Size)
{
    mesh Mesh = {};

    u32 FaceVertices = 3;

    u32 Vertices = Header.FaceElementsCount * FaceVertices; 
    u32 VerticesSize = Vertices * sizeof(vertex_point);

    Mesh.Vertices = (vertex_point *)BufferVertices;
    Mesh.VertexSize = VerticesSize;

    // create 2 temp arrays for vertices/normals we will shrink stack later
    u32 SizeUniqueVertices = Header.VertexCount * sizeof(v3);
    v3 * UniqueVertexArray = (v3 *)PushSize(Arena,SizeUniqueVertices);

    // Pre-load list of unique vertices
    u32 Line = 0;

    for (u32 ci = (Header.VertexStart); 
            (Line < Header.VertexCount); 
            ++Line)
    {
        ci += 2; // fixed
        for (u32 CoordinateIndex = 0;
                CoordinateIndex < FaceVertices;
                ++CoordinateIndex)
        {
            UniqueVertexArray[Line]._V[CoordinateIndex] = (r32)atof(Data + ci);
            AdvanceAfterWs(Data,Size,&ci);
        }
    }
    
    u32 SizeUniqueNormals = Header.VertexNormalCount * sizeof(v3);
    v3 * UniqueNormalArray = (v3 *)PushSize(Arena,SizeUniqueNormals);

    Line = 0;

    for (u32 ci = (Header.VertexNormalStart); 
            (Line < Header.VertexNormalCount); 
            ++Line)
    {
        ci += 3; // fixed
        for (u32 CoordinateIndex = 0;
                CoordinateIndex < FaceVertices;
                ++CoordinateIndex)
        {
            UniqueNormalArray[Line]._V[CoordinateIndex] = (r32)atof(Data + ci);
            AdvanceAfterWs(Data,Size,&ci);
        }
    }

    // Create all vertices indices data
    char * End;
    Line = 0;
    u32 Indice = 0;
    u32 Base10 = 10;
    for (u32 ci = (Header.FaceElementsStart); 
            (Line < Header.FaceElementsCount); 
            ++Line)
    {
        ci += 2; // fixed
        for (u32 VertexIndex = 0;
                VertexIndex < FaceVertices;
                ++VertexIndex)
        {
            i16 IndexVertexP = (i16)strtoimax(Data + ci,&End, Base10) - (i16)1;
            i16 IndexVertexT = (i16)strtoimax(End + 1,&End, Base10) - (i16)1;
            i16 IndexVertexN = (i16)strtoimax(End + 1,&End, Base10) - (i16)1;

            IndexVertexP = IndexVertexP - (i16)OffsetVertexP;
            IndexVertexN = IndexVertexN - (i16)OffsetVertexN;

            Assert(IndexVertexP <= (i16)Header.VertexCount);
            //Assert(IndexVertexT <= ArrayCount(UniqueVertexArray));
            Assert(IndexVertexN <= (i16)Header.VertexNormalCount);

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
    for (u32 i = 0; i < Vertices;++i)
    {
        Log("Pos: %f %f %f Normal: %f %f %f\n",Mesh.Vertices[i].P.x,Mesh.Vertices[i].P.y,Mesh.Vertices[i].P.z,Mesh.Vertices[i].N.x,Mesh.Vertices[i].N.y,Mesh.Vertices[i].N.z);
    }
#endif

    return Mesh;
}

/* ------------------------- END MODEL LOADER EMBEDDED ------------------------- */

void
PushMeshSize(memory_arena * Arena, u32 DataSize, u32 InstanceCount)
{
    u32 TotalSize = (DataSize * InstanceCount);
    u32 Align = RenderGetVertexMemAlign() - 1;
    TotalSize = (TotalSize + Align) &  ~Align;

    Assert((Arena->CurrentSize + TotalSize) < Arena->MaxSize);

    Arena->CurrentSize += TotalSize;
}

THREAD_WORK_HANDLER(LoadMesh)
{
    async_load_mesh * WorkData = (async_load_mesh *)Data;
    Assert(WorkData->ThreadArena);

    game_state * GameState = WorkData->GameState;
    mesh_group * MeshGroup = WorkData->MeshGroup;
    Assert(MeshGroup);

    memory_arena * Arena = &WorkData->ThreadArena->Arena;
    Assert(!MeshGroup->Loaded);

    i32 Result = -1;
    file_contents GetFileResult = GetFileContents(WorkData->Memory, Arena,WorkData->Path);

    if (GetFileResult.Success)
    {
        u32 CurrentByteIndex = 0;
        u32 GPUVertexBufferBaseOffset = WorkData->BaseOffset;
        u8 * BufferVertices = (u8 *)WorkData->BufferVertices;
        // Object file stacks face index position 
        u32 OffsetVertexP = 0;
        u32 OffsetVertexN = 0;

        for (u32 MeshObject = 0;
                    MeshObject < MeshGroup->TotalMeshObjects;
                    ++MeshObject)
        {
            obj_file_header Header = 
                ReadObjFileHeader((const char  *)GetFileResult.Base,CurrentByteIndex, GetFileResult.Size);
            //Logn("Object %s",Header.Name);
            mesh TempMesh   = 
                CreateMeshFromObjHeader(Arena, BufferVertices, Header, (const char  *)GetFileResult.Base, GetFileResult.Size,
                                        OffsetVertexP,OffsetVertexN);

            mesh * Mesh = MeshGroup->Meshes + MeshObject;
            Mesh->Vertices       = TempMesh.Vertices; // vertex_point * Vertices;
            Mesh->VertexSize     = TempMesh.VertexSize; // u32   VertexSize;
            Mesh->Indices        = TempMesh.Indices; // Typedef * Indices;
            Mesh->IndicesSize    = TempMesh.IndicesSize; // u32   IndicesSize;

            // The GPU vertex buffer offset where this will live
            // should be same offset as in our mesh arena
            Mesh->OffsetVertices = GPUVertexBufferBaseOffset;

            // TODO GPU barrier sync
            RenderPushVertexData(Mesh->Vertices,Mesh->VertexSize, Mesh->OffsetVertices);
            //RenderPushVertexData(Mesh->Vertices,Mesh->VertexSize, 1);
            //RenderPushIndexData(&GameState->IndicesArena, Mesh->Indices,Mesh->IndicesSize, 1);

            //
            
            BufferVertices = (BufferVertices + Mesh->VertexSize);
            GPUVertexBufferBaseOffset = (GPUVertexBufferBaseOffset + Mesh->VertexSize);
            CurrentByteIndex = Header.CurrentByteIndex;
            OffsetVertexP += Header.VertexCount;
            OffsetVertexN += Header.VertexNormalCount;
        }

        COMPILER_DONOT_REORDER_BARRIER;
        MeshGroup->Loaded = true;
        MeshGroup->LoadInProcess = false;
    }

#if 0
    v3 dir[3] = {
        {1,0,0},{0,1,0},{0,0,1}
    };

    for (u32 VectorDirIndex = 0;
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

inline u32
StrLen(const char * c)
{
    u32 ci = 0;
    for (;
            (c[ci] != 0);
            ++ci)
    { }
    return ci;
}


inline void
CopyStr(char * DestStr,const char * SrcStr ,u32 Length)
{
    for (u32 ci = 0;
                ci < Length;
                ++ci)
    {
        DestStr[ci] = SrcStr[ci];
    }
}

mesh_group *
GetMesh(game_memory * Memory, game_state * GameState,u32 ID)
{
    mesh_group * MeshGroup = (GameState->Meshes + ID);

    if (!MeshGroup->Loaded && !MeshGroup->LoadInProcess)
    {
        thread_memory_arena * ThreadArena = GetThreadArena(GameState);
        if (ThreadArena)
        {
            MeshGroup->LoadInProcess = true;

            memory_arena * Arena = ThreadBeginArena(ThreadArena);

            async_load_mesh * Data = PushStruct(Arena,async_load_mesh);

            MeshGroup->TotalMeshObjects = MeshObjects[ID];

            // TODO: warning! use pushmeshsize for m64 boundaries memory
            MeshGroup->Meshes = PushArray(&GameState->MeshesArena,MeshGroup->TotalMeshObjects,mesh);

            Data->MeshGroup = MeshGroup;
            Data->Memory    = Memory;                  // game_memory * Memory;
            Data->GameState = GameState;               // game_state * GameState;
            u32 LenPath  = StrLen(MeshPaths[ID]) + 1;
            Data->Path      = (char *)PushSize(Arena,LenPath); // Char_S * Path;
            CopyStr(Data->Path, MeshPaths[ID], LenPath);
            Data->ThreadArena = ThreadArena;

            u32 MeshSize = MeshSizes[ID];
            // For multi thread
            // Get current base for arena
            Data->BaseOffset = GameState->MeshesArena.CurrentSize;
            Data->BufferVertices = GameState->MeshesArena.Base + GameState->MeshesArena.CurrentSize;
            // Increment MeshArena with the known total size required
            PushMeshSize(&GameState->MeshesArena, MeshSize, 1);

            // TODO I don't like this
            // Multi thread mesh load, push arena size based on size
            // of the current requested mesh
            PushSize(&GameState->VertexArena, MeshSize);
            

            //Mesh->OffsetIndices = GameState->IndicesArena.CurrentSize;
            
            Memory->AddWorkToWorkQueue(Memory->RenderWorkQueue , LoadMesh,Data);
        }
    }

    return MeshGroup;
}
