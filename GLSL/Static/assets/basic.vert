varying vec2 pos;

void main()
{
    vec4 outPosition = gl_ModelViewProjectionMatrix * gl_Vertex;
    pos = outPosition.xy;
    gl_Position = outPosition;
}