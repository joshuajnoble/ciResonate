uniform sampler2D samp;
varying vec2 tc;
const float n = 512., h = 1./n;
void main(void) {
vec4 t = texture2D(samp, tc);
t.r -= (texture2D(samp, vec2(tc.r + h, tc.g)).a - t.a)*n;
t.g -= (texture2D(samp, vec2(tc.r, tc.g + h)).a - t.a)*n;
gl_FragColor = t;
}