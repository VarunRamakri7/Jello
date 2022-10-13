#version 400
uniform float time;

in vec2 tex_coord; 

out vec4 fragcolor; //the output color for this fragment    

void main(void)
{   
	fragcolor = vec4(tex_coord, 0.0f, 1.0f); // Color as texture coordinates
}