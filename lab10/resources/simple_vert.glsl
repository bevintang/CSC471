#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 MV;

out vec3 fragN;
out vec4 vertPosition;

void main()
{
	gl_Position = P * MV * vec4(vertPos, 1.0);

	vertPosition = MV * vec4(vertPos, 1.0);
	//fragN = normalize(vertNor);
	fragN = normalize((inverse(transpose(MV)) * vec4(vertNor,0)).xyz);
}
