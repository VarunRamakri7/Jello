#version 440

layout(location = 0) uniform mat4 M;
layout(location = 1) uniform mat4 PV;
layout(location = 2) uniform float time;

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

layout(std140, binding = 2) uniform CameraUniforms {
    vec3 eye;
    vec3 up;
    ivec2 resolution;
    float aspect;
}Camera;

in vec3 pos_attrib; // This variable holds the position of mesh vertices
in vec2 tex_coord_attrib;
in vec3 normal_attrib;  

out vec2 tex_coord;
out vec3 normal;
out vec3 position;
out vec3 light_intensity;

void main(void)
{
	tex_coord = tex_coord_attrib; //send tex_coord to fragment shader
	
	position = vec3(M * vec4(pos_attrib, 1.0)); // World-space vertex position
	normal = normalize(M * vec4(normal_attrib, 0.0f)).xyz;

	gl_Position = PV * vec4(position, 1.0);
}