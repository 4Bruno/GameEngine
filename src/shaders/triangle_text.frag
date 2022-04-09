#version 450

#include "shader_common.h"

layout (location = 0) in interpolants IN;

layout (location = 0) out vec4 outFragColor;

layout (set = 0, binding = 0) uniform SimulationBuffer
{
    simulation_data Data;

} Simulation;

layout (set = 2, binding = 0) uniform sampler2D text1;

void main()
{
    vec4 Color = texture(text1,IN.UV);
    outFragColor = Color * IN.Color;
}
