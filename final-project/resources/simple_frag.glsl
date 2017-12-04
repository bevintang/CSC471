#version 330 core 

uniform vec3 uColor;

in vec3 fragNor;
out vec4 color;

void main()
{
	vec3 normal = normalize(fragNor);
	// Map normal in the range [-1, 1] to color in range [0, 1];
	color = vec4(uColor, 1);
}
