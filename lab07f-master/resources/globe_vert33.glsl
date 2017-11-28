#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 MV;

vec3 source = vec3(-8, 5, 10);
vec3 maxIntensity = vec3(1, 1, 1);

const float AMBIENCE = 0.3;
const vec3 DIFFUSE = vec3(0.6, 0, 0.8);	// purple

out vec3 fragColor;

void main(){
	gl_Position = P * MV * vec4(vertPos, 1.0);

	// Calculate light vector from current position
	vec3 light = source - gl_Position.xyz;
	light = (MV * vec4(light, 0)).xyz;	// transform light along with rotation
	light = normalize(light);	// normalize light vector

	// Calculate Surface Normal vector
	vec3 normal = (MV * vec4(vertNor, 0)).xyz;
	normal = normalize(normal);	// normalize normal vector

	// Calculate color using ambience and diffusion
	// Portion out the total intensity between ambience and diffused light
	vec3 diffLight = DIFFUSE * clamp(dot(normal, light), 0.0, 1.0) * maxIntensity;
	vec3 ambLight = AMBIENCE * diffLight;

	// Set calculated color as color of frag
	fragColor.x = min(ambLight.x + diffLight.x, 1.0);
	fragColor.y = min(ambLight.y + diffLight.y, 1.0);
	fragColor.z = min(ambLight.z + diffLight.z, 1.0);
}