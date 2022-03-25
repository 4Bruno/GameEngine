#version 460

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec4 Color;
layout (location = 3) in vec2 UV;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 TextCoord;

layout ( push_constant ) uniform constants
{
    vec4 Data;
    mat4 RenderMatrix;
    mat4 Model;
    vec4 ColorDebug;

} PushConstants;

layout (set = 0, binding = 0) uniform SimulationBuffer
{
    vec4 AmbientLight;
    vec4 SunlightDirection;
    vec4 SunlightColor;

} SimulationData;

struct objects_data 
{
    mat4 MVP;
    mat4 ModelMatrix;
    vec4 Color;
};

layout (std140, set = 1, binding = 0) readonly buffer objects_buffer
{
    objects_data Objects[]; 
} ObjectsArray;

void main()
{
    mat4 ModelMatrix = ObjectsArray.Objects[gl_BaseInstance].ModelMatrix;
    mat4 MVP = ObjectsArray.Objects[gl_BaseInstance].MVP;
    vec4 Color = ObjectsArray.Objects[gl_BaseInstance].Color;
    outColor = Color;

    gl_Position = MVP * vec4(Position, 1.0f);
}

