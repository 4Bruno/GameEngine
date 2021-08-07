#include "game.h"
#include "mesh.h"
#include <inttypes.h>

void
SkipLine(const char * Data, uint32 Size,uint32 * ci)
{
    uint32 c = *ci;
    for (; (c < Size && Data[c] != '\n') ;++c)
    {
    };
    *ci = ++c;
}

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
// INDEXES VS NOT INDEEXED
#if 0
mesh
CreateMeshFromObjHeader(memory_arena * Arena,obj_file_header Header, const char * Data, uint32 Size)
{
    mesh Mesh = {};

    Mesh.Vertices = PushArray(Arena,Header.VertexCount,vertex_point);
    Mesh.VertexSize = Header.VertexCount*sizeof(vertex_point);

    uint32 Line = 0;
    uint32 FaceVertices = 3;

    for (uint32 ci = (Header.VertexStart); 
            (Line < Header.VertexCount); 
            ++Line)
    {
        ci += 2; // fixed
        for (uint32 CoordinateIndex = 0;
                CoordinateIndex < FaceVertices;
                ++CoordinateIndex)
        {
            Mesh.Vertices[Line].P._V[CoordinateIndex] = (real32)atof(Data + ci);
            AdvanceAfterWs(Data,Size,&ci);
        }
    }

    Line = 0;

    for (uint32 ci = (Header.VertexNormalStart); 
            (Line < Header.VertexNormalCount); 
            ++Line)
    {
        ci += 2; // fixed
        for (uint32 CoordinateIndex = 0;
                CoordinateIndex < FaceVertices;
                ++CoordinateIndex)
        {
            Mesh.Vertices[Line].N._V[CoordinateIndex] = (real32)atof(Data + ci);
            AdvanceAfterWs(Data,Size,&ci);
        }
    }

    Mesh.Indices     = PushArray(Arena,Header.FaceElementsCount*FaceVertices,uint16);
    Mesh.IndicesSize = Header.FaceElementsCount*FaceVertices*sizeof(uint16);

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
            Mesh.Indices[Indice++] = (uint16)strtoimax(Data + ci,&End, Base10) - (uint16)1;
            AdvanceAfterWs(Data,Size,&ci);
        }
    }

#if 1
    for (uint32 i = 0; i < 30;i+=3)
    {
        Log("%u %u %u\n",Mesh.Indices[i], Mesh.Indices[i+ 1],Mesh.Indices[i+ 2]);
    }
#endif

    return Mesh;
}
#else
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
#endif
