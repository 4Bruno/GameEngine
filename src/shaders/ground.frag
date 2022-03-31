#version 450

#define DEBUG 1

layout (location = 0) in vec4 ProjCoord;
layout (location = 1) in vec4 WSCoord;
layout (location = 2) in vec3 ChunkCoord;
#if DEBUG
layout (location = 3) in vec4 inColor;
layout (location = 1) out vec4 outFragColor;
#endif

layout (location = 0) out float outDensity;

void main()
{
    //outFragColor = vec4(1,1,1,1);
#if DEBUG
    outFragColor = inColor;
#endif
    outDensity = -WSCoord.y;
}
