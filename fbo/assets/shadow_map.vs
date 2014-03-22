#version 120                                                                        
                                                                                    
attribute vec3 Position;
attribute vec2 TexCoord;
attribute vec3 Normal;                                                     

varying vec2 TexCoordOut;

                                                                                    
void main()                                                                         
{
    TexCoordOut = TexCoord;
    //gl_Position = gWVP * vec4(Position, 1.0);
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
}
