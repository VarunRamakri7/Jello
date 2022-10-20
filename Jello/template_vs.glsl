#version 440

layout(location = 0) uniform mat4 M;
layout(location = 1) uniform mat4 PV;
layout(location = 2) uniform float time;
layout(location = 3) uniform int pass;

layout(std140, binding = 0) uniform LightUniforms {
	vec4 light_w; // world-space light position
	vec4 bg_color; // Background color
} Light;

layout(std140, binding = 2) uniform CameraUniforms {
    vec3 eye;
    vec3 up;
    ivec2 resolution;
    float aspect;
} Camera;

in vec3 pos_attrib; // This variable holds the position of vertices
in vec2 tex_coord_attrib;
in vec3 normal_attrib;  

out vec2 tex_coord;
out vec3 normal;
out vec3 position;
out vec3 eye_dir;
out vec3 light_dir;

void main(void)
{
	// Assign position depending on pass
	position = (pass == 0) ? pos_attrib : vec3(M * vec4(pos_attrib, 1.0)); // World-space vertex position
	tex_coord = tex_coord_attrib; //send tex_coord to fragment shader
	
	eye_dir = -1.0 * normalize(vec3(M * vec4(pos_attrib, 1.0)));
	light_dir = normalize(Light.light_w).xyz;

	normal = normalize(M * vec4(normal_attrib, 0.0)).xyz;

	gl_Position = (pass == 0) ? vec4(position, 1.0) : PV * vec4(position, 1.0);
}