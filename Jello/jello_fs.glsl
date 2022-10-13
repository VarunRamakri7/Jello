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

out vec4 fragcolor; //the output color for this fragment    

// Calculate blinn-phong shading
vec3 blinnPhong(vec3 pos, vec3 n)
{  
	vec3 ambient = Light.La.xyz * Material.Ka.xyz;
	vec3 s = normalize(Light.light_w.xyz - pos);
	
	float sDotN = max(dot(s, n), 0.0f);
	vec3 diffuse = Material.Kd.xyz * sDotN;
	vec3 spec = vec3(0.0f);
	
	if(sDotN > 0.0f)
	{
	  vec3 v = normalize(-pos);
	  vec3 h = normalize(v + s);
	  spec = Material.Ks.xyz * pow(max(dot(h,n), 0.0), Material.Shininess);
	}
	
	return ambient + Light.Ld.xyz + Light.Ls.xyz * (diffuse + spec);
}

void main(void)
{   
    fragcolor = vec4(blinnPhong(position, normal), 1.0f);

	//fragcolor = vec4(tex_coord, 0.0f, 1.0f); // Color as texture coordinates
}