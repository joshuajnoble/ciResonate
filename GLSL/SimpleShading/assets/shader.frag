#version 120

// this is how we receive the texture
uniform sampler2D tex0;
uniform sampler2D tex1;

uniform float divisor;

varying vec2 texCoordVarying;

void main()
{
    vec2 v = vec2( (1.f + sin(divisor)) / 2.f, texCoordVarying.y);
    gl_FragColor = texture2D(tex0, v);
}