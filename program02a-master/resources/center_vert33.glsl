#version 330 core
layout(location = 0) in vec3 vertPos;

uniform mat4 P;
uniform mat4 MV;

void main(){
    gl_Position = P * MV * vec4(0.4 * vertPos, 1.0);
}
