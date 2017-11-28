#version  330 core
layout(location = 0) in vec3 vertPos;
uniform mat4 P;
uniform mat4 MV;
uniform mat4 view;

out vec2 texCoord;

void main()
{
   gl_Position = P * view * MV * vec4(vertPos, 1);
	texCoord = (vertPos.xy+vec2(1, 1))/2.0;
}
