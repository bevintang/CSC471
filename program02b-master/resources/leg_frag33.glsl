#version 330 core

in vec3 fragNor;

out vec4 color;

void main()
{
	vec3 normal = normalize(fragNor);
	vec3 Ncolor = 0.3 * normal + 0.7;	// map color from range [-1,1] to [0,1]

	// Make color mostly peach
	color = vec4(Ncolor, 1.0);
	color.g = color.r - 0.3;
	color.b = color.g - 0.2;
}
