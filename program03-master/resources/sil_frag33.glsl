#version 330 core

uniform vec3 uLight;
uniform vec3 eye;

vec3 light = vec3(600*uLight.x, 1000 * uLight.y, 1000 * uLight.z);

in vec3 normal;		// vertex normal
in vec3 vertPosition;
out vec3 color;

void main()
{
	vec3 nNormal = normalize(normal);
	vec3 eyeDir = normalize(eye - vertPosition);
	float cosTheta = dot(nNormal, eyeDir);

	// Set color to black if the angle between the normal and eye
	// about 90 degrees
	if (cosTheta < 0.3 && cosTheta > -0.3){
		color = vec3(0, 0, 0);
	}
	// Otherwise, color white
	else{
		color = vec3(1, 1, 1);
	}
}