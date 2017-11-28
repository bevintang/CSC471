#version 330 core

in vec2 texCoord;
uniform sampler2D texBuf;
uniform float pov;

out vec4 color;

void main(){
   	color = texture(texBuf, texCoord );	// give color
   	if (pov == 1){
   		color = vec4(max(max(color.r, color.g), color.b));
   	}
}
