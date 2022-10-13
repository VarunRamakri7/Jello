#version 440

layout(location = 0) uniform mat4 M;
layout(location = 1) uniform mat4 PV;
layout(location = 2) uniform float time;

in vec3 pos_attrib; // This variable holds the position of mesh vertices
in vec2 tex_coord_attrib;
in vec3 normal_attrib;  

out vec2 tex_coord;
out vec3 normal;
out vec3 position;

void main(void)
{
	tex_coord = tex_coord_attrib; //send tex_coord to fragment shader
	
	position = vec3(M * vec4(pos_attrib, 1.0)); // World-space vertex position
	normal = vec3(M * vec4(normal_attrib, 0.0f)); // World-space normal vector
	
	gl_Position = PV * M * vec4(pos_attrib, 1.0);
}