#version 330 core
out vec3 color;

uniform vec2 center;
uniform vec2 uWindowSize;

vec2 trueCenter;
float whiteness = 0;

void main(){
	whiteness = distance(gl_FragCoord.xy, center) / (uWindowSize.y * 0.52);
	color = vec3(1.0 + whiteness, 0.3 + whiteness, 0.4);
}
