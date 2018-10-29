#version 330 core

uniform vec3 uLight;
uniform vec3 eye;
uniform vec3 AMBIENCE;
uniform vec3 DIFFUSE;
uniform vec3 SPECULAR;
uniform float SHINE;			// shine: lower => more area

vec3 source = uLight;

in vec3 normal;		// surface normal
in vec3 vertPosition;
in vec3 eyePos;
out vec4 color;

const vec3 maxIntensity = vec3(1.0, 1.0, 1.0);


void main()
{
	// Calculate light vector from current position
	vec3 light = source - vertPosition;
	light = normalize(light);		// normalize light vector

	// Calculate eye direction and H-vector
	vec3 eyeDir = normalize(eye - vertPosition);
	vec3 H = normalize(light + eyeDir);

	// Normalize normal
	vec3 nNormal = normalize(normal);

	// Calculate color using ambience and diffusion
	// gPortion out the total intensity between ambience and diffused light
	vec3 diffLight = DIFFUSE * clamp(dot(nNormal, light), 0.0, 1.0) * maxIntensity;
	vec3 ambLight = AMBIENCE * maxIntensity;
	vec3 specLight = SPECULAR * clamp(pow(dot(nNormal, H), SHINE), 0, 1) * maxIntensity;
 
	// Set calculated color as color of frag
	color = vec4 (clamp(diffLight + ambLight + specLight, 0, 1), 1);
}