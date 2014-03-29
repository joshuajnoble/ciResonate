
#version 120

// Simple Fresnel Silhouette approximation
///////////////////////////////////////////
varying vec3 vN;
varying vec3 vV;
varying vec3 ReflectDir;

void main() {
	gl_Position = ftransform();
	vV = (gl_ModelViewMatrix * gl_Vertex).xyz;
    vN = gl_NormalMatrix * gl_Normal;
    ReflectDir	= reflect( vV, vN );
}