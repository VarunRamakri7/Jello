#version 440
uniform float time;

layout(std140, binding = 0) uniform LightUniforms {
  vec4 Position; // Light position in eye coords.
  vec3 La; // Ambient light intesity
  vec3 L; // Diffuse and specular light intensity
} Light;

layout(std140, binding = 1) uniform MaterialUniforms {
  vec3 Ka; // Ambient reflectivity
  vec3 Kd; // Diffuse reflectivity
  vec3 Ks; // Specular reflectivity
  float Shininess; // Specular shininess factor
} Material;

in vec2 tex_coord;
in vec4 position;
in vec3 normal;

out vec4 fragcolor; //the output color for this fragment    

// Calculate blinn-phong shading
vec3 blinnPhong(vec3 position, vec3 n)
{  
	vec3 ambient = Light.La * Material.Ka;
	vec3 s = normalize(Light.Position.xyz - position);
	
	float sDotN = max(dot(s, n), 0.0f);
	vec3 diffuse = Material.Kd * sDotN;
	vec3 spec = vec3(0.0f);
	
	if(sDotN > 0.0f)
	{
	  vec3 v = normalize(-position);
	  vec3 h = normalize(v + s);
	  spec = Material.Ks * pow(max(dot(h,n), 0.0), Material.Shininess);
	}
	
	return ambient + Light.L * (diffuse + spec);
}

void main(void)
{   
    fragcolor = vec4(blinnPhong(position.xyz, normal), 1.0f);

	//fragcolor = vec4(tex_coord, 0.0f, 1.0f); // Color as texture coordinates
}