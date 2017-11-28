#version 330 core
out vec3 color;

in vec3 vertColor;
uniform vec2 center;
uniform float uTime;
uniform vec2 uWindowSize;

vec2 trueCenter;

void main()
{
	float radius = uWindowSize.x / 20; // pixel coordiantes
	trueCenter.x = center.x + radius * cos(uTime);
	trueCenter.y = center.y + radius * sin(uTime);
	float whiteness = distance(gl_FragCoord.xy, trueCenter) / (uWindowSize.x / 4);
	color = vertColor + whiteness;

   // Make upper half blue
   //if (gl_FragCoord.y > uWindowSize.y/2){
   //   color = vec3(0.0, 0.3, 1.0);
   //}

   if (distance(gl_FragCoord.xy, trueCenter) < 20.0){
      discard;
   }
}
