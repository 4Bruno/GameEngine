#version 460

#include "shader_common.h"

VERTEX_INPUT_DECLARATION

INTERPOLANTS_OUT

SIMULATION_BUFFER

OBJECTS_BUFFER


void main()
{
    vec4 Color = ObjectsArray.Objects[gl_InstanceIndex].Color;
    float Depth = (ObjectsArray.Objects[gl_InstanceIndex].ViewMatrix * vec4(Position,1.0f)).z;

    outIP.Color = Color;
    outIP.Normal = Normal;
    outIP.Pos = Position;
    outIP.Depth = Depth;
    outIP.UV = UV;
    
    outImageIndex = ObjectsArray.Objects[gl_InstanceIndex].ImageIndex;

    //gl_Position = MVP * vec4(Position, 1.0f);
    vec4 Pos = vec4(Position * 0.5f, 1.0f);
    //Pos.z = ObjectsArray.Objects[gl_InstanceIndex].ModelMatrix[0][2];
    gl_Position = Pos;
}

