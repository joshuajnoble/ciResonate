
uniform float time;
uniform vec2 res;
uniform float divisor;
uniform vec2 rotation;

varying vec2 pos;

void main()
{
    vec2 p=(pos-res)/divisor;
    p = p * vec2(rotation);
    gl_FragColor = sin(vec4(8,4,2,0)*(dot(p,p)-time)) + vec4(1., 1., 1., 1.);
}