#version 120

// this is how we receive the texture
uniform sampler2DRect tex0;

varying vec2 texCoordVarying;

void main()
{
    gl_FragColor = texture2DRect(tex0, texCoordVarying);
    //gl_FragColor = vec4(texCoordVarying.x,texCoordVarying.y,0,1);
}