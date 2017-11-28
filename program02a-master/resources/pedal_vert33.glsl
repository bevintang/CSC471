#version 330 core
layout(location = 0) in vec3 vertPos;

uniform mat4 P;
uniform mat4 MV;
uniform float uTime;

float scalar = 1;
vec3 truePos = vertPos;

void main(){
	if (vertPos.z == 1){	// if outer vertex of pedal
		truePos.x = vertPos.x * (3 + cos(uTime)) * 0.8;
		truePos.y = vertPos.y * (3 + cos(uTime)) * 0.8;
	}
	else{
		truePos.x = vertPos.x * (5.25 + sin(uTime + 3.14159/4)) * 0.3;
		truePos.y = vertPos.y * (5.25 + sin(uTime + 3.14159/4)) * 0.3;
	}
    gl_Position = P * MV * vec4(0.5 * truePos, 1.0);
}
