#version 440

layout(location = 2) uniform float time;
layout(location = 3) uniform int pass;

layout(std140, binding = 0) uniform LightUniforms {
   vec4 light_w; // world-space light position
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

    return (Material.base_color + Material.spec_factor * spec * Material.spec_color);
}

void main(void)
{
    switch(pass)
    {
        case 0: // Render Background
            fragcolor = vec4(0.9f, 0.45f, 0.25f, 1.0f);
            break;
        case 1: // Render mesh back faces and store eye-space depth
            break;
        case 2: // Render front faces, compute eye-space depth
            break;
        default:
            fragcolor = min(HackTransparency(), vec4(1.0));
            break;
    }

	//fragcolor = vec4(normal, 1.0f); // Color as normals
}