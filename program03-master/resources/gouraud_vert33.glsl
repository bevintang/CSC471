#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 MV;
uniform vec3 uLight;
uniform vec3 eye;
uniform vec3 AMBIENCE;
uniform vec3 DIFFUSE;
uniform vec3 SPECULAR;
uniform float SHINE;

vec3 light = vec3(600*uLight.x, 1000 * uLight.y, 1000 * uLight.z);

const vec3 maxIntensity = vec3(1, 1, 1);

out vec3 fragColor;

void main(){
	gl_Position = P * MV * vec4(vertPos, 1.0);

	// Calculate light vector from current position
	light = light - (MV*vec4(vertPos, 1)).xyz;
	light = normalize(light);		// normalize light vector

	// Calculate Surface Normal vector
	vec3 normal = (MV * vec4(vertNor, 0)).xyz;
	normal = normalize(normal);		// normalize normal vector

	// Calculate eyeDirection and H-vector
	vec3 vertPosition = (MV * vec4(vertPos, 1)).xyz;
	vec3 eyeDir = normalize(eye - vertPosition);
	vec3 H = normalize(light + eyeDir);

	// Calculate color using ambience and diffusion
	// Portion out the total intensity between ambience and diffused light
	vec3 diffLight = DIFFUSE * dot(normal, light) * maxIntensity;
	vec3 ambLight = AMBIENCE * maxIntensity;
	vec3 specLight = SPECULAR * pow(dot(normal, H), SHINE) * maxIntensity;

	// Set calculated color as color of frag
	fragColor = clamp(diffLight + ambLight + specLight, 0, 1);
}