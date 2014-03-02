uniform sampler2D camTex;
uniform sampler2D xTex;
uniform sampler2D yTex;

void main()
{
//	float xVal	= texture2D( xTex, gl_TexCoord[0].st ).r;
//	float yVal	= texture2D( yTex, gl_TexCoord[0].st ).r;
	vec3 camCol = texture2D( camTex, gl_TexCoord[0].st ).rgb;
	
//	vec3 col	= vec3( xVal, yVal, 0.0 ) * 0.5 + 0.5;
	
	gl_FragColor.rgb	= camCol;
	gl_FragColor.a		= 1.0;
}