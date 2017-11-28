#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D texBuf;
uniform vec2 dir;

void main(){
   	color = vec4(texture( texBuf, texCoord ).rgb, 1);

   	// Flip colors
   	float red = color.r;
   	float green = color.g;
   	float blue = color.b;
	color.r = blue;
	color.g = red;
	color.b = green;
}
