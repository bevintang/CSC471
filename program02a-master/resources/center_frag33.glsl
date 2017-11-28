#version 330 core
out vec3 color;

uniform vec2 center;
uniform vec2 uWindowSize;

vec2 trueCenter;
float whiteness = 0;

void main()
{
	whiteness = distance(gl_FragCoord.xy, center) / (uWindowSize.x / 1.55);
	color = vec3(0.9, 0.6, 0.3) + whiteness;
}
