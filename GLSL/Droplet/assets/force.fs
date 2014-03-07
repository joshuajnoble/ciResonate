uniform sampler2D samp;
uniform float c;
varying vec2 tc;
const float h = 1.0/512.0;

void main(void) {
    vec4 t = texture2D(samp, tc);
    t.g += c*(t.b + texture2D(samp, vec2(tc.r, tc.g + h)).b );
    gl_FragColor = t;
}