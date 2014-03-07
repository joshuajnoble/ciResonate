uniform sampler2D samp;
varying vec2 tc;
const float h = 1./512., h2 = 1./1024.;
void main(void) {
vec4 t = texture2D(samp, tc);
t.a =
(texture2D(samp, vec2(tc.r - h, tc.g)).a +
texture2D(samp, vec2(tc.r + h, tc.g)).a +
texture2D(samp, vec2(tc.r, tc.g - h)).a +
texture2D(samp, vec2(tc.r, tc.g + h)).a -
// (t.r - texture2D(samp, vec2(tc.r - h, tc.g)).r + is unstable !?
// t.g - texture2D(samp, vec2(tc.r, tc.g - h)).g) *h) *.25;
(texture2D(samp, vec2(tc.r + h, tc.g)).r -
texture2D(samp, vec2(tc.r - h, tc.g)).r +
texture2D(samp, vec2(tc.r, tc.g + h)).g -
texture2D(samp, vec2(tc.r, tc.g - h)).g) *h2) *.25;
gl_FragColor = t;
}