uniform mat4 matrix;
uniform mat4 mvpMatrix;
uniform vec3 eyePos;

varying vec4 vVertex;
varying vec3 vNormal;
varying vec4 vColor;
varying vec3 vEyeDir;

void main()
{
	gl_TexCoord[0]	= gl_MultiTexCoord0;
	
	vVertex			= vec4( gl_Vertex );
	vNormal			= normalize( gl_Normal );
	vColor			= vec4( gl_Color );
	vEyeDir			= normalize( eyePos - vVertex.xyz );
	
	gl_Position		= mvpMatrix * gl_Vertex;
}