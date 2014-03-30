#version 120

#extension GL_EXT_gpu_shader4 : require

uniform sampler2D		dataTex;
uniform float			texWidth;
uniform float			texHeight;
uniform float			soundDataSize;
uniform float			spread;
uniform float			spreadOffset;
uniform float			time;
uniform vec4            tintColor;


void main()
{
	vec2 texPos;
	int binN		= int( ( spreadOffset + gl_MultiTexCoord0.x * spread ) * soundDataSize );
	texPos.x		= float( mod( binN, int(texWidth) ) + 0.5 ) / texWidth;
	texPos.y		= float( binN / int(texWidth) + 0.5 ) / texHeight;
    vec4 col        = texture2D( dataTex, texPos );
	gl_FrontColor   = tintColor * col;
    gl_FrontColor.a = col.r;
    gl_Position     = ftransform();
}
