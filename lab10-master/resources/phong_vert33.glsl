#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 MV;
uniform mat4 view;

out vec3 normal;
out vec3 vertPosition;
out vec3 eyePos;

void main(){
	gl_Position = P * view * MV * vec4(vertPos, 1.0);
	// Calculate Surface Normal vector
	normal = (MV * vec4(vertNor, 0)).xyz;

	// Calculate vertPos after transformation
	vertPosition = (MV * vec4(vertPos, 1)).xyz;
}