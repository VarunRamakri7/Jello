#version 410
uniform mat4 PVM;
uniform float time;

in vec3 pos_attrib; // This variable holds the position of mesh vertices
in vec2 tex_coord_attrib;
in vec3 normal_attrib;  

out vec2 tex_coord;
out vec3 normal;
out vec4 position;

void main(void)
{
	position = PVM * vec4(pos_attrib, 1.0); //transform vertices and send result into pipeline
	tex_coord = tex_coord_attrib; //send tex_coord to fragment shader
	normal = normal_attrib;

	gl_Position = position;
}