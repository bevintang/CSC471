#version 330 core
uniform sampler2D Texture0;

in vec2 vTexCoord;
in float dCo;
out vec4 Outcolor;

void main() {
  vec4 texColor0 = texture(Texture0, vTexCoord);
 
  	Outcolor = dCo*texColor0;
  
	float r = Outcolor.r;
	float g = Outcolor.g;
	float b = Outcolor.b;

	Outcolor.r = g;
	Outcolor.g = b;
	Outcolor.b = r;
}

