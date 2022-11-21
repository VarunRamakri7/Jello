#version 440

layout(binding = 0) uniform sampler2D fbo_tex; 
layout(binding = 1) uniform sampler2D depth_tex;

layout(location = 0) uniform mat4 M;
layout(location = 2) uniform float time;
layout(location = 3) uniform int pass;

layout(std140, binding = 2) uniform LightUniforms {
    vec4 light_w; // world-space light position
    vec4 bg_color; // Background color
} Light;

layout(std140, binding = 3) uniform MaterialUniforms {
  vec4 base_color; // base color
  vec4 spec_color; // Specular Color
  vec4 absorption; // x,y,z are absorbption, z is specular factor
} Material;

layout(std140, binding = 4) uniform CameraUniforms {
    vec4 eye;
    vec4 up;
    vec4 resolution;
} Camera;

in VertexData
{
	vec2 tex_coord;
	vec3 normal; // World-space unit normal vector
	vec3 position; // World-space vertex position
	vec3 eye_dir; // Normalized eye-direction
	vec3 light_dir; // Normalized light direction
	vec4 depth;
} inData;

out layout(location = 0) vec4 fragcolor; //the output color for this fragment    
out layout(location = 1) vec4 depthVal; // Write depth to Color attachment 1

const float near = 0.1f;
const float far = 100.0f;

//const vec3 jello_absorb = vec3(0.8, 0.8, 0.1); // Amount of each color absorbed by the object
const float reflectivity = 0.5f; // Reflectivity of object
const float n_air = 1.0029f;
const float n_obj = 1.125f;

// From https://blog.demofox.org/2017/01/09/raytracing-reflection-refraction-fresnel-total-internal-reflection-and-beers-law/
float FresnelReflectAmount (float n1, float n2, vec3 normal, vec3 incident)
{
        // Schlick aproximation
        float r0 = (n1-n2) / (n1+n2);
        r0 *= r0;
        float cosX = -dot(normal, incident);
        if (n1 > n2)
        {
            float n = n1/n2;
            float sinT2 = n*n*(1.0-cosX*cosX);
            // Total internal reflection
            if (sinT2 > 1.0)
                return 1.0;
            cosX = sqrt(1.0-sinT2);
        }
        float x = 1.0-cosX;
        float ret = r0+(1.0-r0)*x*x*x*x*x;
 
        // adjust reflect multiplier for object reflectivity
        ret = (reflectivity + (1.0-reflectivity) * ret);
        return ret;
}

vec4 HackTransparency()
{
    vec3 reflect_dir = -reflect(inData.light_dir, inData.normal);

    float fresnel = FresnelReflectAmount(n_air, n_obj, inData.normal, inData.light_dir);
    //fresnel -= 1.0f;

    float spec = max(dot(inData.eye_dir, reflect_dir * fresnel), 0.0);
    spec *= spec;

    return (Material.base_color + Material.spec_color * spec * Material.absorption.z) * Light.bg_color;
}

void main(void)
{
    vec2 uv = vec2(gl_FragCoord.x / Camera.resolution.x, gl_FragCoord.y / Camera.resolution.y); // Get uv coordinate

    switch(pass)
    {
        case 0: // Render Background
            fragcolor = Light.bg_color;
            depthVal = inData.depth; // Store eye-space depth
            break;
        case 1: // Render mesh back faces and store eye-space depth
            if(!gl_FrontFacing)
            {
                depthVal = normalize(inData.depth); // Store eye-space depth

                fragcolor = Material.base_color;
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
                vec3 thickness = abs(normalize(inData.depth) - texture(depth_tex, uv)).xyz; // Compute thickness from front face depth and back face depth

                // Get refracted background color
                //vec4 ref_bg_color = Light.bg_color * inverse(transpose(M));

                // Compute Beer's Law for final color
                vec3 color = min(HackTransparency(), vec4(1.0)).xyz;
                vec3 absorb = exp(-Material.absorption.xyz * thickness);

                fragcolor = vec4(color * absorb, 1.0);
                //fragcolor = ref_bg_color;
            }
            else
            {
                discard; // Discard back facing fragments
            }
            break;
        case 4: // Textured Quad
                fragcolor = texture(fbo_tex, uv); // Display FBO texture
                //fragcolor = texture(depth_tex, uv); // Display FBO texture
            break;
        default:
            fragcolor = min(HackTransparency(), vec4(1.0));
            break;
    }

	//fragcolor = vec4(inData.normal, 1.0f); // Color as normals
}