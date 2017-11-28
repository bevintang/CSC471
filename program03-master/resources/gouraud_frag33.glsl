#version 330 core
in vec3 fragColor;

uniform vec3 eye;

out vec3 color;

void main()
{
	color = fragColor;
}