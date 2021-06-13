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
        if ( Data[ci] == ' ' )
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
CreateMeshFromObjHeader(memory_arena * Arena,obj_file_header Header, const char * Data, uint32 Size)
{
    mesh Mesh = {};

    Mesh.Vertices = PushArray(Arena,Header.VertexCount,vertex_point);
    Mesh.VertexSize = Header.VertexCount*sizeof(vertex_point);

    uint32 Line = 0;

    for (uint32 ci = (Header.VertexStart); (Line < Header.VertexCount); ++Line)
    {
        ci += 2; // fixed
        Mesh.Vertices[Line].P.x = (real32)atof(Data + ci);
        AdvanceAfterWs(Data,Size,&ci);
        Mesh.Vertices[Line].P.y = (real32)atof(Data + ci);
        AdvanceAfterWs(Data,Size,&ci);
        Mesh.Vertices[Line].P.z = (real32)atof(Data + ci);
        SkipLine(Data,Size,&ci);
    }

    Mesh.Indices     = PushArray(Arena,Header.FaceElementsCount*4,uint16);
    Mesh.IndicesSize = Header.FaceElementsCount*2*sizeof(uint16);

    char * End;

    Line = 0;
    uint32 Indice = 0;
    uint32 Base10 = 10;
    for (uint32 ci = (Header.FaceElementsStart); (Line < Header.FaceElementsCount); ++Line)
    {
        ci += 2; // fixed
        Mesh.Indices[Indice++] = (uint16)strtoimax(Data + ci,&End, Base10);
        AdvanceAfterWs(Data,Size,&ci);
        Mesh.Indices[Indice++] = (uint16)strtoimax(Data + ci,&End, Base10);
        AdvanceAfterWs(Data,Size,&ci);
        Mesh.Indices[Indice++] = (uint16)strtoimax(Data + ci,&End, Base10);
        AdvanceAfterWs(Data,Size,&ci);
        Mesh.Indices[Indice++] = (uint16)strtoimax(Data + ci,&End, Base10);

        SkipLine(Data,Size,&ci);
    }

    return Mesh;
}
