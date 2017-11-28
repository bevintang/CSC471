#version 330 core

in vec3 normal;
out vec3 color;

void main()
{
	color = normalize(normal);
}