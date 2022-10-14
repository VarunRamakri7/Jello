#version 440

layout(location = 2) uniform float time;

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

void main(void)
{   
    vec4 color;

    vec3 reflect_dir = -reflect(light_dir, normal);

    float spec = max(dot(eye_dir, reflect_dir), 0.0);
    spec *= spec;

    color = Material.base_color + Material.spec_factor * spec * Material.spec_color;

    fragcolor = min(color, vec4(1.0));

	//fragcolor = vec4(normal, 1.0f); // Color as normals
}