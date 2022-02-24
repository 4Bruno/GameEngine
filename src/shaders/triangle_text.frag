#version 450

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec2 inTextCoord;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inPosition;

layout (location = 0) out vec4 outFragColor;

layout (set = 0, binding = 0) uniform SimulationBuffer
{
    vec4 AmbientLight;
    vec4 SunlightDirection;
    vec4 SunlightColor;

} SimulationData;

layout (set = 2, binding = 0) uniform sampler2D text1;

void main()
{
    vec3 Color = texture(text1,inTextCoord).xyz;
    outFragColor = vec4(Color,1.0f);
    //outFragColor = inColor;

    // Determine the blend weights for the 3 planar projections. 
    // N_orig is the vertex-interpolated normal vector. 
    vec3 blend_weights = abs( inNormal.xyz );   
    // Tighten up the blending zone: 
    blend_weights = (blend_weights - 0.2) * 7;
    blend_weights = max(blend_weights, 0);
      
    // Force weights to sum to 1.0 (very important!) 
    blend_weights /= (blend_weights.x + blend_weights.y + blend_weights.z ).xxx;
 
    // Now determine a color value and bump vector for each of the 3 
    // projections, blend them, and store blended results in these two 
    // vectors: 
    vec4 blended_color;
 
    // .w hold spec value 
    vec3 blended_bump_vec;
    { 
        // Compute the UV coords for each of the 3 planar projections. 
        // tex_scale (default ~ 1.0) determines how big the textures appear. 
        float tex_scale = 0.1f;
        vec2 coord1 = inPosition.yz * tex_scale;
        vec2 coord2 = inPosition.xz * tex_scale;
        vec2 coord3 = inPosition.xy * tex_scale;

        // This is where you would apply conditional displacement mapping. 
        //if (blend_weights.x > 0) coord1 = . . . 
        //if (blend_weights.y > 0) coord2 = . . . 
        //if (blend_weights.z > 0) coord3 = . . . 
        // Sample color maps for each projection, at those UV coords. 
        vec4 col1 = texture(text1,coord1);
        vec4 col2 = texture(text1,coord2);
        vec4 col3 = texture(text1,coord3);

#if 0
        // Sample bump maps too, and generate bump vectors. 
        // (Note: this uses an oversimplified tangent basis.) 
        vec2 bumpFetch1 = bumpTex1.Sample(coord1).xy - 0.5;
        vec2 bumpFetch2 = bumpTex2.Sample(coord2).xy - 0.5;
        vec2 bumpFetch3 = bumpTex3.Sample(coord3).xy - 0.5;
        vec3 bump1 = vec3(0, bumpFetch1.x, bumpFetch1.y);
        vec3 bump2 = vec3(bumpFetch2.y, 0, bumpFetch2.x);
        vec3 bump3 = vec3(bumpFetch3.x, bumpFetch3.y, 0);
#endif
  
        // Finally, blend the results of the 3 planar projections. 
        blended_color = col1.xyzw * blend_weights.xxxx + col2.xyzw * blend_weights.yyyy + col3.xyzw * blend_weights.zzzz;
    } 

    // Apply bump vector to vertex-interpolated normal vector. 
    //vec3 N_for_lighting = normalize(inNormal + blended_bump);

    outFragColor= blended_color;

}
