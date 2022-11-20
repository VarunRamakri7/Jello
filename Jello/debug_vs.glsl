#version 440            

layout(location = 0) uniform mat4 M;
layout(location = 1) uniform mat4 PV;

in vec3 pos_attrib;


void main(void)
{
    //transform vertices and send result into pipeline
    gl_Position = PV*M*vec4(pos_attrib, 1.0); 
}