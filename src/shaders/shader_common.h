#define INTERPOLANTS_IN \
layout (location = 0) in interpolants IN;\
layout (location = 5) in flat int ImageIndex;

#define INTERPOLANTS_OUT \
layout (location = 0) out interpolants outIP;\
layout (location = 5) out flat int outImageIndex;

#define SIMULATION_BUFFER \
layout (set = 0, binding = 0) uniform SimulationBuffer\
{\
    simulation_data Data;\
\
} Simulation;

#define OBJECTS_BUFFER \
layout (std140, set = 1, binding = 0) readonly buffer objects_buffer\
{\
    objects_data Objects[]; \
} ObjectsArray;\

#define VERTEX_INPUT_DECLARATION \
layout (location = 0) in vec3 Position;\
layout (location = 1) in vec3 Normal;\
layout (location = 2) in vec4 Color;\
layout (location = 3) in vec2 UV;

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
    int ImageIndex;
};

struct simulation_data
{
    vec4 AmbientLight;
    vec4 SunlightDirection;
    vec4 SunlightColor;
};


struct mesh_push_constant
{
    vec4 DebugColor;
    int ImageIndex;
};
