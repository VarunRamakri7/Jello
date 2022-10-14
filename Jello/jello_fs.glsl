#version 440

layout(std140, binding = 0) uniform LightUniforms {
   vec4 La; // Ambient light color
   vec4 Ld; // Diffuse light color
   vec4 Ls; // Specular light color
   vec4 light_w; // world-space light position
} Light;

layout(std140, binding = 1) uniform MaterialUniforms {
  vec4 Ka; // Ambient reflectivity
  vec4 Kd; // Diffuse reflectivity
  vec4 Ks; // Specular reflectivity
  float Shininess; // Specular shininess factor
} Material;

layout(location = 2) uniform float time;

in vec2 tex_coord;
in vec3 position;
in vec3 normal;
in vec3 light_intensity;

out vec4 fragcolor; //the output color for this fragment    

void main(void)
{   
	fragcolor = vec4(normal, 1.0f); // Phong shading
}