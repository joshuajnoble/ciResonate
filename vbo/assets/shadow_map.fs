#version 120                                                                        
                                                                                    
varying vec2 TexCoordOut;
uniform sampler2DRect gShadowMap;

void main()                                                                         
{                                                                                   
    float Depth = texture2DRect(gShadowMap, TexCoordOut).x;
    Depth = 1.0 - (1.0 - Depth) * 25.0;                                             
    gl_FragColor = vec4(Depth);
}
