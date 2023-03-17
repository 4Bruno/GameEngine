#version 450

#include "shader_common.h"

layout (location = 0) flat in interpolants IN;

layout (location = 0) out vec4 outFragColor;

layout (set = 0, binding = 0) uniform SimulationBuffer
{
    vec4 AmbientLight;
    vec4 SunlightDirection;
    vec4 SunlightColor;

} SimulationData;

void main()
{
    outFragColor = IN.Color;
}
