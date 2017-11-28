#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 MV;

vec3 source = vec3(0, 0, -1);
vec3 maxIntensity = vec3(1, 1, 1);

const float AMBIENCE = 0.2;
const vec3 DIFFUSE = vec3(0.8, 0, 1.0);	// purple

out vec3 fragColor;

void main(){
	gl_Position = P * MV * vec4(vertPos, 1.0);

	// Calculate light vector from current position
	vec3 light = gl_Position.xyz - source;
	light = (MV * vec4(light, 0)).xyz;	// transform light along with rotation
	light = normalize(light);	// normalize light vector

	// Calculate Surface Normal vector
	vec3 normal = (MV * vec4(vertNor, 0)).xyz;
	normal = normalize(normal);	// normalize normal vector

	// Calculate color using ambience and diffusion
	// Portion out the total intensity between ambience and diffused light
	vec3 ambLight = AMBIENCE * maxIntensity;
	vec3 diffLight = DIFFUSE * clamp(dot(normal, light), 0.0, 1.0) * maxIntensity;

	// Set calculated color as color of frag
	fragColor = ambLight + diffLight;
}