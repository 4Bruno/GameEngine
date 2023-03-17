#version 460

#include "shader_common.h"
VERTEX_INPUT_DECLARATION

INTERPOLANTS_OUT

SIMULATION_BUFFER

OBJECTS_BUFFER

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
    mat4 ModelMatrix = ObjectsArray.Objects[gl_BaseInstance].ModelMatrix;
    mat4 MVP = ObjectsArray.Objects[gl_BaseInstance].MVP;
    vec4 Color = ObjectsArray.Objects[gl_BaseInstance].Color;
    float AmbientLight = Simulation.Data.AmbientLight.w;

    float SpecularLight = CalculateSpecularLight(ModelMatrix);

    outIP.Color = (AmbientLight + SpecularLight) * Color;
    outIP.Normal = Normal;
    outIP.Pos = Position;
    outIP.Depth = 1.0f;
    outIP.UV = UV;

    outImageIndex = ObjectsArray.Objects[gl_BaseInstance].ImageIndex;

    gl_Position = MVP * vec4(Position, 1.0f);
}

