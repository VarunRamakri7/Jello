#version 440

layout(location = 0) uniform mat4 M;
layout(location = 1) uniform mat4 PV;
layout(location = 2) uniform float time;
layout(location = 3) uniform int pass;

uniform sampler2D fboTex;

layout(std140, binding = 2) uniform LightUniforms {
	vec4 light_w; // world-space light position
	vec4 bg_color; // Background color
} Light;

layout(std140, binding = 4) uniform CameraUniforms {
    vec4 eye;
    vec4 up;
    vec4 resolution;
} Camera;

in vec3 pos_attrib; // This variable holds the position of vertices
in vec2 tex_coord_attrib;
in vec3 normal_attrib;  

out VertexData
{
	vec2 tex_coord;
	vec3 normal; // World-space unit normal vector
	vec3 position; // World-space vertex position
	vec3 eye_dir; // Normalized eye-direction
	vec3 light_dir; // Normalized light direction
	vec4 depth;
} outData;

const float near = 0.1f;
const float far = 100.0f;

const vec4 quad[4] = vec4[] (vec4(-1.0, 1.0, 0.0, 1.0), 
							 vec4(-1.0, -1.0, 0.0, 1.0), 
							 vec4( 1.0, 1.0, 0.0, 1.0), 
							 vec4( 1.0, -1.0, 0.0, 1.0) );

void main(void)
{
	// For all passes except QUAD
	if (pass != 4)
	{
		// Assign position depending on pass
		outData.position = (pass == 0) ? pos_attrib : vec3(M * vec4(pos_attrib, 1.0)); // World-space vertex position
		outData.tex_coord = tex_coord_attrib; // Send tex_coord to fragment shader
	
		outData.eye_dir = -1.0 * normalize(outData.position);
		outData.light_dir = normalize(Light.light_w).xyz;

		outData.normal = normalize(M * vec4(normal_attrib, 0.0)).xyz;

		outData.depth =  (pass == 0) ? vec4(1.0f) : (PV * vec4(pos_attrib, 1.0)); // Send eye-space depth

		gl_Position = (pass == 0) ? vec4(outData.position, 1.0) : PV * vec4(outData.position, 1.0);
	}
	else // Draw fullscreen quad
	{
		gl_Position = quad[ gl_VertexID ]; //get clip space coords out of quad array
		outData.tex_coord = 0.5 * (quad[ gl_VertexID ].xy + vec2(1.0)); 
	}
}