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
//  float idx       = ( ( cos(time) * gl_TexCoord[0].x ) * 0.5 * spread + spreadOffset ) * soundDataSize;
//  float idx       = gl_TexCoord[0].x + spreadOffset ) * soundDataSize * spread;
//  idx             = mod( idx, soundDataSize );
	
	vec2 texPos;
	int binN		= int( ( spreadOffset + gl_TexCoord[0].x * spread ) * soundDataSize );
	texPos.x		= float( mod( binN, int(texWidth) ) + 0.5 ) / texWidth;
	texPos.y		= float( binN / int(texWidth) + 0.5 ) / texHeight;
	gl_FragColor	= tintColor * texture2D( dataTex, texPos );
}
