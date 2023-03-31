#version 460

#include "shader_common.h"

VERTEX_INPUT_DECLARATION

INTERPOLANTS_OUT

SIMULATION_BUFFER

OBJECTS_BUFFER


void main()
{
    objects_data Object = ObjectsArray.Objects[gl_InstanceIndex];
    vec4 Color = Object.Color;
    float Depth = (Object.ViewMatrix * vec4(Position,1.0f)).z;

    outIP.Color = Color;
    outIP.Normal = Normal;
    outIP.Pos = Position;
    outIP.Depth = Depth;

    float SubTextSize = 1.0f / 10.0f;
    outIP.UV.x = (UV.x + Object.TextSample.x) * SubTextSize;
    outIP.UV.y = (UV.y + Object.TextSample.y) * SubTextSize;
    
    outImageIndex = Object.ImageIndex;

    //vec4 Pos = vec4(Position * 0.5f, 1.0f);
    //Pos.z = ObjectsArray.Objects[gl_InstanceIndex].ModelMatrix[0][2];
    //gl_Position = Pos;
    gl_Position = Object.ModelMatrix * vec4(Position, 1.0f);
}

