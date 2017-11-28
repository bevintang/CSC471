#version 330 core

uniform vec2 uWindowSize;
uniform vec2 center;

out vec3 color = vec3(0.2, 0.0, 1.0);

float blackness = 0.0;


void main()
{
	blackness = distance(gl_FragCoord.xy, center) / (uWindowSize.x);
	color = vec3(0.2, 0.1, 0.7) - blackness;
}
