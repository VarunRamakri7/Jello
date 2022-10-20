#version 440

layout(location = 2) uniform float time;
layout(location = 3) uniform int pass;

//layout(location = 4) uniform sampler2D depthTex;

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

in vec2 tex_coord;
in vec3 position;
in vec3 normal;
in vec3 eye_dir;
in vec3 light_dir;

out vec4 fragcolor; //the output color for this fragment    

vec4 HackTransparency()
{
    vec3 reflect_dir = -reflect(light_dir, normal);

    float spec = max(dot(eye_dir, reflect_dir), 0.0);
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
                
            }
            break;
        case 2: // Render front faces, compute eye-space depth
            if(gl_FrontFacing)
            {
                
            }
            break;
        default:
            fragcolor = min(HackTransparency(), vec4(1.0));
            break;
    }

    //vec2 res = gl_FragCoord.xy / vec2(Camera.resolution.xy);
    //fragcolor = texture(depthTex, res);

	//fragcolor = vec4(normal, 1.0f); // Color as normals
}