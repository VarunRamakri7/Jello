#version 440

layout(location = 0) uniform mat4 M;
layout(location = 1) uniform mat4 PV;
layout(location = 2) uniform float time;
layout(location = 3) uniform int pass;

uniform sampler2D fboTex;

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

out VertexData
{
	vec2 tex_coord;
	vec3 normal;
	vec3 position;
	vec3 eye_dir;
	vec3 light_dir;
	float depth;
} outData;

const float near = 0.1f;
const float far = 100.0f;

const vec4 quad[4] = vec4[] (vec4(-1.0, 1.0, 0.0, 1.0), 
							 vec4(-1.0, -1.0, 0.0, 1.0), 
							 vec4( 1.0, 1.0, 0.0, 1.0), 
							 vec4( 1.0, -1.0, 0.0, 1.0) );

void main(void)
{
	// Assign position depending on pass
	outData.position = (pass == 0) ? pos_attrib : vec3(M * vec4(pos_attrib, 1.0)); // World-space vertex position
	outData.tex_coord = tex_coord_attrib; // Send tex_coord to fragment shader
	
	outData.eye_dir = -1.0 * normalize(vec3(M * vec4(pos_attrib, 1.0)));
	outData.light_dir = normalize(Light.light_w).xyz;

	outData.normal = normalize(M * vec4(normal_attrib, 0.0)).xyz;

	outData.depth =  (pass == 0) ? 1.0f : (PV * vec4(pos_attrib, 1.0)).z; // Send eye-space depth

	gl_Position = (pass == 0) ? vec4(outData.position, 1.0) : PV * vec4(outData.position, 1.0);
}