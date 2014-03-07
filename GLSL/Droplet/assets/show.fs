uniform sampler2D samp;
varying vec2 tc;
void main(void) {
	float rg = 1. + texture2D(samp, tc).b;
	gl_FragColor = vec4(rg, rg, 1. + rg, 1.);
}