#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 MV;

vec3 maxIntensity = vec3(1, 1, 1);

out vec3 source = vec3(-100, 600, 800);	// light source -- pixel coordinates
out vec3 normal;

void main(){
	gl_Position = P * MV * vec4(vertPos, 1.0);
	//source =  (MV*vec4(800, 800, 800, 0)).xyz;
	// Calculate Surface Normal vector
	normal = (MV * vec4(vertNor, 0)).xyz;
}