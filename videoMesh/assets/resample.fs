
#version 120

uniform sampler2DRect tex;
uniform sampler2D dataTex;
varying vec2 texCoord;

void main()
{
    vec2 multiplier = vec2(640,480);
    vec2 additive = vec2(320,240);
    
	vec4 d = texture2D(dataTex, (texCoord/multiplier));
	gl_FragColor = texture2DRect(tex, (d.xy * multiplier));
    //gl_FragColor = d;
}
