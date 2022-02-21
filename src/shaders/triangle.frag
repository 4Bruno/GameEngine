#version 450

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec2 textCoord;

layout (location = 0) out vec4 outFragColor;

void main()
{
    //outFragColor = inColor;
    outFragColor = vec4(textCoord.x, textCoord.y, 0.5f,1.0f);
}
