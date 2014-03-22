#version 120

uniform float time;
varying vec3 TexCoord;

void main()
{
	TexCoord =  vec3(gl_MultiTexCoord0.st, (time/100.0));
	vec4 v = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;

	TexCoord.x *= mix(0, 1, sin(time));
	TexCoord.y *= mix(0, 1, cos(time));

	gl_Position = v;
}
