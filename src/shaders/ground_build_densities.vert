#version 460

//#include "master.h"

/*
 * Draw 2 triangles covering entire render target for each Block (1 Block = (n*4+1) Voxels)
 * (Note: this will actually take range -1 to 1)
 * T1, vertices: [0,0],[0,1],[1,1]
 * T2, vertices: [0,0],[1,1],[1,0]
 */
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV;

#define DEBUG 1

layout (location = 0) out vec4 outProjCoord;
layout (location = 1) out vec3 outChunkCoord;
layout (location = 2) out int  outInstanceIndex;
#if DEBUG 
layout (location = 3) out vec4 outColor;
#endif

layout (std140, set = 0, binding = 0) uniform ViewPortData 
{
    vec4 viewportDim;
} ViewPort;

/* LOD (Level of detail) of voxel being generated */
layout (std140, set = 0, binding = 1) uniform voxel_lod
{
    float VoxelDim;
    float VoxelDimMinusOne;
    vec2 wsVoxelSize;
    vec2 wsChunkSize;
    vec2 InvVoxelDim;
    vec2 InvVoxelDimMinusOne;
    float Margin;
    float VoxelDimPlusMargins;
    float VoxelDimPlusMarginsMinusOne;
    float _Padding01;
    vec2 InvVoxelDimPlusMargins;
    vec2 InvVoxelDimPlusMarginsMinusOne;
} VoxelLOD;

struct chunk_data
{
    vec3 wsChunkPos; //wsCoord of lower-left corner
    float opacity;
};

// Uniform buffer for each chunk
layout (set = 1, binding = 0) readonly buffer ChunkArrayBuffer
{
    chunk_data Chunks[];
} ChunkArray;

layout ( push_constant ) uniform constants
{
    mat4 MVP;

} PushConstants;

vec3 
rot(vec3 coord, mat4 mat)
{
  return vec3( dot(vec3(mat[0][0], mat[0][1],mat[0][2]), coord),   // 3x3 transform,
                 dot(vec3(mat[1][0], mat[1][1],mat[1][2]), coord),   // no translation
                 dot(vec3(mat[2][0], mat[2][1],mat[2][2]), coord) );
}

void
main()
{
    // [-1..1]
    vec4 projCoord = vec4(inPosition.xy, 0.5, 1);
    
    // chunkCoord is in [0..1] range
    vec3 chunkCoord = 
        vec3( inUV, 
              gl_InstanceIndex * VoxelLOD.InvVoxelDimPlusMargins.x); // ([0..32] / 41) < 1); 

    // Cover voxels beyond chunk for complex shadows/light
    // ([0..1] * 41) - 4 = [-4..37] where 33 is the VoxelDim and Margin is 4
    // [-4..37] / 33 = [-0.12..1.12]
    vec3 extChunkCoord = (chunkCoord*VoxelLOD.VoxelDimPlusMargins.x - VoxelLOD.Margin)*VoxelLOD.InvVoxelDim.x;
                                
    // [-0.12..1.12] * ChunkSizeLOD (1 for close, 2 for medium, 4 for far)
    vec3 ws = ChunkArray.Chunks[gl_BaseInstance].wsChunkPos + extChunkCoord*VoxelLOD.wsChunkSize.x;

    outProjCoord = projCoord;
    outChunkCoord = chunkCoord;
    outInstanceIndex = gl_InstanceIndex;
#if DEBUG
    gl_Position = PushConstants.MVP * vec4(ws, 1.0f);
    vec3 P = ChunkArray.Chunks[gl_BaseInstance].wsChunkPos;
    vec3 Color = vec3(mod(P.x,3) / 3.0f + 0.33f,mod(P.y,2) / 2.0f + 0.5f,mod(P.z,4) / 4.0f + 0.25f);
    outColor = vec4(Color,1);
#else
    gl_Position = vec4(ws, 1.0f);
#endif
}
