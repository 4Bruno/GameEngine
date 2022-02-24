#version 450

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec2 inTextCoord;

layout (location = 0) out vec4 outFragColor;

void main()
{
    //outFragColor = vec4(inTextCoord.x,inTextCoord.y,0.5f,1.0f);
    outFragColor = inColor;
}
