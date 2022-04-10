#version 460

#include "shader_common.h"

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec4 Color;
layout (location = 3) in vec2 UV;

layout (location = 0) out interpolants outIP;

layout (set = 0, binding = 0) uniform SimulationBuffer
{
    simulation_data Data;

} Simulation;

layout (std140, set = 1, binding = 0) readonly buffer objects_buffer
{
    objects_data Objects[]; 
} ObjectsArray;

void main()
{
    mat4 ModelMatrix = ObjectsArray.Objects[gl_InstanceIndex].ModelMatrix;
    mat4 MVP = ObjectsArray.Objects[gl_InstanceIndex].MVP;
    vec4 Color = ObjectsArray.Objects[gl_InstanceIndex].Color;
    float Depth = (ObjectsArray.Objects[gl_InstanceIndex].ViewMatrix * vec4(Position,1.0f)).z;

    outIP.Color = Color;
    outIP.Normal = Normal;
    outIP.Pos = Position;
    outIP.Depth = Depth;
    outIP.UV = UV;

    gl_Position = MVP * vec4(Position, 1.0f);
}

