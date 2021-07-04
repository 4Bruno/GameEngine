#version 450

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec4 Color;

layout (location = 0) out vec4 outColor;

layout ( push_constant ) uniform constants
{
    vec4 Data;
    mat4 RenderMatrix;
    mat4 ViewRotationMatrix;
    mat4 Model;
    vec3 SourceLight;
    bool IsLightSource;
    vec4 ColorDebug;
} PushConstants;

void main()
{
    float AmbientLight = 0.2f;
    vec3 FragPos = vec3(PushConstants.Model * vec4(Position,1.0f));
    vec3 N = normalize(mat3(transpose(inverse(PushConstants.Model))) * Normal);
    float Intensity = 1.0f;
    vec3 Ray = PushConstants.SourceLight - FragPos;
    Intensity = max(dot(Ray,N),0.0f);
    float Dist = 1.0f / length(Ray);
    //outColor = PushConstants.ColorDebug * max(AmbientLight,min(Intensity*Dist, 1.0f));
    outColor = vec4(N,1.0f);
    //outColor = vec4(Ray,1.0f);
    gl_Position = PushConstants.RenderMatrix * vec4(Position, 1.0f);
}

