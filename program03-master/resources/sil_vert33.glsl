#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 MV;
uniform vec3 AMBIENCE;
uniform vec3 DIFFUSE;
uniform vec3 SPECULAR;
uniform float SHINE;

vec3 amb = AMBIENCE;
vec3 diff = DIFFUSE;
vec3 spec = SPECULAR;
float sh = SHINE;

out vec3 normal;
out vec3 vertPosition;

void main(){
	gl_Position = P * MV * vec4(vertPos, 1.0);
	// Calculate Surface Normal vector
	normal = (MV * vec4(vertNor, 0)).xyz;

	// Calculate vertPos after transformation
	vertPosition = (MV * vec4(vertPos, 1)).xyz;
}