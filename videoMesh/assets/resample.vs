#version 120 

varying vec2 texCoord;

void main()
{
    // need to get this to 640/480
    vec4 p = ftransform();
    //v_texcoord = vec2(a_texcoord.s, 1.0 - a_texcoord.t);
    texCoord = vec2(p.x + 320, 240 - p.y);
    gl_Position = p;
}
