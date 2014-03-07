uniform sampler2D samp;
varying vec2 tc;
const float h = 1./512., dt = .001, tau = .5*dt/h;
void main(void) {
vec2 D = -tau*vec2(
texture2D(samp, tc).r + texture2D(samp, vec2(tc.r - h, tc.g)).r,
texture2D(samp, tc).g + texture2D(samp, vec2(tc.r, tc.g - h)).g );
vec2 Df = floor(D), Dd = D - Df;
vec2 tc1 = tc + Df*h;
vec3 new =
(texture2D(samp, tc1).rgb*(1. - Dd.g) +
texture2D(samp, vec2(tc1.r, tc1.g + h)).rgb*Dd.g)*(1. - Dd.r) +
(texture2D(samp, vec2(tc1.r + h, tc1.g)).rgb*(1. - Dd.g) +
texture2D(samp, vec2(tc1.r + h, tc1.g + h)).rgb*Dd.g)*Dd.r;
gl_FragColor = vec4( new, texture2D(samp, tc).a );
}