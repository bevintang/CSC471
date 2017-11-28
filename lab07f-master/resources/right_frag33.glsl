#version 330 core

in vec3 source;		// light source
in vec3 normal;		// surface normal
out vec3 color;

vec3 maxIntensity = vec3(1.0, 1.0, 1.0);

const vec3 AMBIENCE = vec3(0.15, 0, 0.2);
const vec3 DIFFUSE = vec3(0.6, 0, 0.8);		// purple

void main()
{
	// Calculate light vector from current position
	vec3 light = source - gl_FragCoord.xyz;
	light = normalize(light);		// normalize light vector

	// Re-normalize normal
	vec3 nNormal = normalize(normal);

	// Calculate color using ambience and diffusion
	// gPortion out the total intensity between ambience and diffused light
	vec3 diffLight = DIFFUSE * clamp(dot(nNormal, light), 0.0, 1.0) * maxIntensity;
	vec3 ambLight = AMBIENCE * maxIntensity;

	// Set calculated color as color of frag
	color.x = clamp(ambLight.x + diffLight.x, 0, 0.8);
	color.y = clamp(ambLight.y + diffLight.y, 0, 1);
	color.z = clamp(ambLight.z + diffLight.z, 0, 1);
}