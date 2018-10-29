#version 330 core

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shinny;
uniform vec3 camera;
uniform vec3 lightSrc;

in vec4 vertPosition;
in vec3 fragN;
layout(location = 0) out vec4 color;

void main()
{
	vec3 L = normalize(lightSrc - vertPosition.xyz);
	vec3 N = normalize(fragN);
	vec3 eyedir = normalize(camera - vertPosition.xyz);
	vec3 H = normalize(L + eyedir);
	vec3 iLight = vec3(1.0, 1.0, 1.0);
	vec3 specularLight = specular * pow(clamp(dot(N, H), 0, 1), shinny) * iLight;
	vec3 lightdiffuse = diffuse * clamp(dot(N, L), 0.0, 1.0) * iLight;
	vec3 ambientLight = ambient * iLight;
	color = vec4(clamp(lightdiffuse + ambientLight + specularLight, 0.0, 1.0), 1.0);
}
