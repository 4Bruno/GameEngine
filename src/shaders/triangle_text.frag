#version 460
#extension GL_ARB_separate_shader_objects : enable

#include "shader_common.h"

INTERPOLANTS_IN

layout (location = 0) out vec4 outFragColor;

SIMULATION_BUFFER

layout (set = 2, binding = 0) uniform sampler samp;
layout (set = 2, binding = 1) uniform texture2D textures[10];

void main()
{
    vec4 Color = texture(sampler2D(textures[ImageIndex], samp),IN.UV);
    //outFragColor = vec4(1.0f);//Color * IN.Color;
    outFragColor = vec4(1.0f, 1.0f, 1.0f, Color.a);
}
