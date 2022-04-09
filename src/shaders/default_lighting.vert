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

float
CalculateSpecularLight(mat4 ModelMatrix)
{
    vec3 FragPos = vec3(ModelMatrix * vec4(Position,1.0f));
    vec3 N = normalize(mat3(transpose(inverse(ModelMatrix))) * Normal);
    vec3 FragToLightSrc = vec3(Simulation.Data.SunlightDirection) - FragPos;
    float FragToLightSrcLength = 1.0f / length(FragToLightSrc);
    FragToLightSrc = FragToLightSrc * FragToLightSrcLength;
    float SpecularLight = min(max(dot(N,FragToLightSrc),0.0f) * (FragToLightSrcLength*10.0f),0.8f);

    return SpecularLight;
}

void main()
{
    mat4 ModelMatrix = ObjectsArray.Objects[gl_InstanceIndex].ModelMatrix;
    mat4 MVP = ObjectsArray.Objects[gl_InstanceIndex].MVP;
    vec4 Color = ObjectsArray.Objects[gl_InstanceIndex].Color;

    float AmbientLight = Simulation.Data.AmbientLight.w;

    float SpecularLight = CalculateSpecularLight(ModelMatrix);

    outIP.Color = (AmbientLight + SpecularLight) * Color;
    outIP.Normal = Normal;
    outIP.Pos = Position;
    outIP.Depth = 1.0f;
    outIP.UV = UV;

    gl_Position = MVP * vec4(Position, 1.0f);
}

