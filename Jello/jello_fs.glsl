#version 440

layout(location = 2) uniform float time;
layout(location = 3) uniform int pass;

layout(binding = 0) uniform sampler2D fbo_tex; 
layout(binding = 1) uniform sampler2D depth_tex;

layout(std140, binding = 0) uniform LightUniforms {
    vec4 light_w; // world-space light position
    vec4 bg_color; // Background color
} Light;

layout(std140, binding = 1) uniform MaterialUniforms {
  vec4 base_color; // base color
  vec4 spec_color; // Specular Color
  float spec_factor; // Specular factor
} Material;

layout(std140, binding = 2) uniform CameraUniforms {
    vec3 eye;
    vec3 up;
    ivec2 resolution;
    float aspect;
} Camera;

in VertexData
{
	vec2 tex_coord;
	vec3 normal;
	vec3 position;
	vec3 eye_dir;
	vec3 light_dir;
	float depth;
} inData;

out layout(location = 0) vec4 fragcolor; //the output color for this fragment    
out layout(location = 1) float depthVal; // Write depth to Color attachment 1

vec4 HackTransparency()
{
    vec3 reflect_dir = -reflect(inData.light_dir, inData.normal);

    float spec = max(dot(inData.eye_dir, reflect_dir), 0.0);
    spec *= spec;

    return (Material.base_color + Material.spec_factor * spec * Material.spec_color) * Light.bg_color;
}

void main(void)
{
    switch(pass)
    {
        case 0: // Render Background
            fragcolor = Light.bg_color;
            break;
        case 1: // Render mesh back faces and store eye-space depth
            if(!gl_FrontFacing)
            {
                // Store eye-space depth
                depthVal = inData.depth;

                fragcolor = vec4(0.85f, 0.25f, 0.25f, 1.0f);
            }
            else
            {
                discard; // Discard front facing fragments
            }
            break;
        case 2: // Render front faces, compute eye-space depth
            if(gl_FrontFacing)
            {
                // Compute front face depth

                // Compute thickness

                // Get refracted background color

                // Compute Beer's Law for final color

                //fragcolor = vec4(0.25f, 0.25f, 0.85f, 1.0f);
            }
            else
            {
                discard; // Discard back facing fragments
            }
            break;
        case 4:
            fragcolor = min(HackTransparency(), vec4(1.0));
            break;
        default:
            fragcolor = min(HackTransparency(), vec4(1.0));
            break;
    }

	//fragcolor = vec4(inData.normal, 1.0f); // Color as normals
}