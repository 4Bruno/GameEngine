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
    vec3 SourceLight;
    bool IsLightSource;
    vec4 ColorDebug;
} PushConstants;

void main()
{
    gl_Position = PushConstants.RenderMatrix * vec4(Position, 1.0f);
    vec3 N = vec3(PushConstants.ViewRotationMatrix * vec4(Normal,1.0f));
    float Intensity = 1.0f;
    if (!PushConstants.IsLightSource)
    {
        vec3 SourceLight = vec3(PushConstants.RenderMatrix * vec4(PushConstants.SourceLight,1.0f));
        vec3 Ray = vec3(gl_Position) - SourceLight;
        float Reflection = dot(-Ray,N);
        Intensity = 0.0f;
        if (Reflection > 0.0f)
        {
            Intensity = Reflection;
        }
    }
    outColor = PushConstants.ColorDebug * Intensity;
    //outColor = vec4(Normal,1.0f);
}

