#version 450

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec4 Color;

layout (location = 0) out vec4 outColor;

layout ( push_constant ) uniform constants
{
    vec4 Data;
    mat4 RenderMatrix;
} PushConstants;

void main()
{
    gl_Position = PushConstants.RenderMatrix * vec4(Position, 1.0f);
    outColor = Color;
}

