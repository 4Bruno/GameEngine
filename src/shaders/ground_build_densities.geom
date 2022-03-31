#version 460

#define DEBUG 1
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout (location = 0) in vec4 ProjCoord[];
layout (location = 1) in vec3 ChunkCoord[];
layout (location = 2) in int  InstanceIndex[];
#if DEBUG
layout (location = 3) in vec4 inColor[];
#endif

layout (location = 0) out vec4  outProjCoord;
layout (location = 1) out vec4  outWSCoord;
layout (location = 2) out vec3  outChunkCoord;
#if DEBUG
layout (location = 3) out vec4  outColor;
#endif

void main()
{
    for (int v=0; v<3; v++) 
    {
        outProjCoord = ProjCoord[v];
        outChunkCoord = ChunkCoord[v];
        gl_Position = gl_in[v].gl_Position;
        gl_Layer = InstanceIndex[v];
#if DEBUG
        outColor = inColor[v];
#endif
        EmitVertex();
    }
    EndPrimitive();
    for (int v=0; v<3; v++) 
    {
        outProjCoord = ProjCoord[v];
        outChunkCoord = ChunkCoord[v];
        gl_Position = gl_in[v].gl_Position + vec4(0,5,0,0);
        gl_Layer = InstanceIndex[v];
#if DEBUG
        outColor = inColor[v];
#endif
        EmitVertex();
    }
    EndPrimitive();
}
