#version 120

attribute vec4 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_texCoord;

varying vec3 v_g_normal;
varying vec2 v_g_texCoord;

void main()
{
	v_g_normal = a_normal;		
	v_g_texCoord = a_texCoord;
	gl_Position = a_vertex;
}    