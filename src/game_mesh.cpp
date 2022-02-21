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

struct parsed_obj_file_result
{
    vertex_point * Vertices;
    u16 *          Indices;

    u32 VertexSize;
    u32 IndicesSize;

    char Name[100];
};

parsed_obj_file_result
CreateMeshFromObjHeader(memory_arena * Arena,obj_file_header Header, const char * Data, u32 Size,
                        u32 OffsetVertexP, u32 OffsetVertexN)
{
    parsed_obj_file_result Result = {};

    u32 FaceVertices = 3;

    u32 Vertices = Header.FaceElementsCount * FaceVertices; 
    u32 VerticesSize = Vertices * sizeof(vertex_point);

    // pre-allocate size of vertices
    u8 * BufferVertices = PushSize(Arena,VerticesSize);

    // Use same arena to temporary allocate
    // unique list of verticesP and normals
    BeginTempArena(Arena,1);

    Result.Vertices = (vertex_point *)BufferVertices;
    Result.VertexSize = VerticesSize;

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
            Result.Vertices[Indice].P = UniqueVertexArray[IndexVertexP];
            // TextCoord
            // TODO
            // N
            Result.Vertices[Indice].N = UniqueNormalArray[IndexVertexN];

            AdvanceAfterWs(Data,Size,&ci);
            ++Indice;
        }
    }

#if 0
    for (u32 i = 0; i < Vertices;++i)
    {
        Log("Pos: %f %f %f Normal: %f %f %f\n",Result.Vertices[i].P.x,Result.Vertices[i].P.y,Result.Vertices[i].P.z,Result.Vertices[i].N.x,Result.Vertices[i].N.y,Result.Vertices[i].N.z);
    }
#endif

    EndTempArena(Arena,1);

    return Result;
}

/* ------------------------- END MODEL LOADER EMBEDDED ------------------------- */


u32
PushMeshSize(memory_arena * Arena, u32 DataSize, u32 InstanceCount)
{
    u32 OffsetBeforeUpdate = Arena->CurrentSize;

    u32 TotalSize = (DataSize * InstanceCount);
    u32 Align = RenderGetVertexMemAlign() - 1;
    TotalSize = (TotalSize + Align) &  ~Align;

    Assert((Arena->CurrentSize + TotalSize) < Arena->MaxSize);

    Arena->CurrentSize += TotalSize;

    return OffsetBeforeUpdate;
}

void
PrepareArenaForGPUAlignment(memory_arena * Arena)
{
    u32 Align = RenderGetVertexMemAlign() - 1;
    u8 * BaseAddr = ((u8 *)Arena->Base + Arena->CurrentSize);
    memory_aligned_result AlignedMem = AlignMemoryAddress((void *)BaseAddr,Align);

    Arena->CurrentSize += AlignedMem.Delta;
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
        u32 VertexBufferBeginOffset = WorkData->BaseOffset;
        // Object file stacks face index position 
        u32 OffsetVertexP = 0;
        u32 OffsetVertexN = 0;
        u32 TotalMeshObjVerticesBytes = 0;

        BeginTempArena(Arena,1);

        PrepareArenaForGPUAlignment(Arena);

        u8 * BeginBufferVertices = Arena->Base + Arena->CurrentSize;
        u8 * BufferVertices = BeginBufferVertices;

#if 0
            for (u32 MeshObject = 1; MeshObject < 2; ++MeshObject)
#else
            for (u32 MeshObjectIndex = 0;
                        MeshObjectIndex < MeshGroup->TotalMeshObjects;
                        ++MeshObjectIndex)
#endif
        {
            obj_file_header Header = 
                ReadObjFileHeader((const char  *)GetFileResult.Base,CurrentByteIndex, GetFileResult.Size);
            //Logn("Object %s",Header.Name);
            //if (MeshObjectIndex <= 1) {
            parsed_obj_file_result MeshObj = 
                CreateMeshFromObjHeader(Arena, Header, (const char  *)GetFileResult.Base, GetFileResult.Size,
                        OffsetVertexP,OffsetVertexN);

            u32 Align = RenderGetVertexMemAlign() - 1;
            // Align to gpu boundaries
            u32 MeshObjSizeAligned = (MeshObj.VertexSize + Align) &  ~Align;

            u32 DeltaMeshObjSizeAligned = MeshObjSizeAligned - MeshObj.VertexSize;
            // zero array aligned
            if (DeltaMeshObjSizeAligned)
            {
                Memset(BufferVertices + MeshObj.VertexSize,0,DeltaMeshObjSizeAligned);
                Arena->CurrentSize += (MeshObjSizeAligned - MeshObj.VertexSize);
            }

            mesh * Mesh = MeshGroup->Meshes + MeshObjectIndex;
            Mesh->VertexSize     = MeshObjSizeAligned; // u32   VertexSize;
            Mesh->IndicesSize    = MeshObj.IndicesSize; // u32   IndicesSize;

            // The GPU vertex buffer offset where this will live
            // should be same offset as in our mesh arena
            Mesh->OffsetVertices = VertexBufferBeginOffset;

            TotalMeshObjVerticesBytes += MeshObjSizeAligned;

            BufferVertices = (BufferVertices + MeshObjSizeAligned);
            VertexBufferBeginOffset = (VertexBufferBeginOffset + MeshObjSizeAligned);
            Assert(((VertexBufferBeginOffset + Align) & ~Align) == VertexBufferBeginOffset);
            //}
            CurrentByteIndex = Header.CurrentByteIndex;
            OffsetVertexP += Header.VertexCount;
            OffsetVertexN += Header.VertexNormalCount;
        }


        //Logn("Actual mesh size %i", TotalMeshObjVerticesBytes);
        // TODO: GPU barrier sync
        RenderPushVertexData(BeginBufferVertices, TotalMeshObjVerticesBytes, WorkData->BaseOffset);
        //RenderPushVertexData(Mesh->Vertices,Mesh->VertexSize, 1);
        //RenderPushIndexData(&GameState->IndicesArena, Mesh->Indices,Mesh->IndicesSize, 1);

        EndTempArena(Arena,1);

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

    TempArenaSanityCheck(Arena);
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

mesh_group
GetMeshInfo(mesh_id MeshID)
{
    mesh_group MeshGroup = {};
    MeshGroup.TotalMeshObjects = MeshObjects[MeshID.ID];
    return MeshGroup;
}

mesh_group *
GetMesh(game_memory * Memory, game_state * GameState,mesh_id MeshID)
{

    Assert(IS_VALID_MESHID(MeshID.ID));

    mesh_group * MeshGroup = (GameState->Meshes + MeshID.ID);
    
    Assert(IS_NOT_NULL(MeshGroup));

    if (!MeshGroup->Loaded && !MeshGroup->LoadInProcess)
    {
        Assert(IS_NULL(MeshGroup->Meshes));
        thread_memory_arena * ThreadArena = GetThreadArena(GameState);
        if (IS_NOT_NULL(ThreadArena))
        {
            MeshGroup->LoadInProcess = true;

            memory_arena * Arena = ThreadBeginArena(ThreadArena);

            async_load_mesh * Data = PushStruct(Arena,async_load_mesh);

            MeshGroup->TotalMeshObjects = MeshObjects[MeshID.ID];

            MeshGroup->Meshes = PushArray(&GameState->MeshesArena,MeshGroup->TotalMeshObjects,mesh);

            Data->MeshGroup = MeshGroup;
            Data->Memory    = Memory;                  // game_memory * Memory;
            Data->GameState = GameState;               // game_state * GameState;
            u32 LenPath  = StrLen(MeshPaths[MeshID.ID]) + 1;
            Data->Path      = (char *)PushSize(Arena,LenPath); // Char_S * Path;
            CopyStr(Data->Path, MeshPaths[MeshID.ID], LenPath);
            Data->ThreadArena = ThreadArena;

            u32 MeshSize = MeshSizes[MeshID.ID];
            u32 MeshObjs = MeshObjects[MeshID.ID];
            // worst case all boundaries falls in next alignment byte
            MeshSize = MeshSize + (MeshObjs * (RenderGetVertexMemAlign() - 2));

            //Logn("Worst case scenario mesh size %i", MeshSize);
            // calculate vertex arena new size in worst case scenario
            // to avoid new any mesh overlapping
            Data->BaseOffset = PushMeshSize(&GameState->Renderer.VertexArena, MeshSize, 1);

            Memory->AddWorkToWorkQueue(Memory->RenderWorkQueue , LoadMesh,Data);
        }
        // use default 0 while loading others
        MeshGroup = GameState->Meshes + 0;
    }

    return MeshGroup;
}

mesh_id
Mesh(u32 ID)
{
    Assert(ID < ArrayCount(MeshObjects));
    mesh_id MeshID = {ID};
    return MeshID;
}
