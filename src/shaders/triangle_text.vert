#version 450

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
    vec3 SourceLight;
    float Dummy;
    vec4 ColorDebug;

} PushConstants;

void main()
{
    vec3 FragPos = vec3(PushConstants.Model * vec4(Position,1.0f));
    vec3 N = normalize(mat3(transpose(inverse(PushConstants.Model))) * Normal);
    float Intensity = 1.0f;
    vec3 FragToLightSrc = PushConstants.SourceLight - FragPos;
    float FragToLightSrcLength = 1.0f / length(FragToLightSrc);
    FragToLightSrc = FragToLightSrc * FragToLightSrcLength;
    float AmbientLight = 0.2f;
    float SpecularLight = min(max(dot(N,FragToLightSrc),0.0f) * (FragToLightSrcLength*10.0f),0.8f);
    //if (PushConstants.SourceLight.y == 0.0f)
    vec3 ColorDebug = vec3(PushConstants.ColorDebug);
    if (ColorDebug.x == 1.0f && ColorDebug.y == 1.0f && ColorDebug.z == 1.0f)
    {
        outColor = vec4(1.0,1.0f,1.0f,1.0f);
    }
    else
    {
        outColor = (AmbientLight + SpecularLight) * PushConstants.ColorDebug;
    }
    //outColor = vec4(N,1.0f);
    //outColor = PushConstants.ColorDebug;
    //outColor = vec4(FragToLightSrc,1.0f);
    //outColor = PushConstants.ColorDebug * clamp(Position.y,0.2f,1.0f);
    //outColor = PushConstants.ColorDebug * clamp(Position.y,0,1.0f) * (AmbientLight + SpecularLight);
    //outColor = vec4(1.0,1.0f,1.0f,1.0f);
    gl_Position = PushConstants.RenderMatrix * vec4(Position, 1.0f);
    TextCoord = UV;
}

