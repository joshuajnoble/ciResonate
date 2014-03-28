#version 120

#extension GL_EXT_gpu_shader4 : require


void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
}