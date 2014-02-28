#version 120

uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;

void main()
{
    mat4 modelViewProjection = u_projectionMatrix * u_modelViewMatrix;
	gl_Position = modelViewProjection * gl_Vertex;;
}    