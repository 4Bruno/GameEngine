struct interpolants
{
  vec3  Pos;     // World-space vertex position
  vec3  Normal;  // World-space vertex normal
  vec4  Color;   // Linear-space color
  vec2  UV;
  float Depth;   // Z coordinate after applying the view matrix (larger = further away)
};

struct objects_data 
{
    mat4 MVP;
    mat4 ModelMatrix;
    mat4 ViewMatrix;
    vec4 Color;
};

struct simulation_data
{
    vec4 AmbientLight;
    vec4 SunlightDirection;
    vec4 SunlightColor;
};
