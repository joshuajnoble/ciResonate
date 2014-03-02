uniform sampler2D captureTex;
uniform vec2 screenSize;
uniform vec2 pos;

varying vec4 vVertex;
varying vec3 vNormal;
varying vec4 vColor;
varying vec3 vEyeDir;

void main()
{
	float ppEyeDiff		= dot( vNormal, vEyeDir );
	if( ppEyeDiff < 0.0 ) discard;
	float ppEyeFres		= pow( 1.0 - ppEyeDiff, 2.0 );
	
	vec3 lightPos		= vec3( screenSize * 0.5, 400.0 );
	vec3 lightDir		= lightPos - vVertex.xyz;
	float lightDist		= length( lightDir );
	float lightDistPer	= min( lightDist/800.0, 1.0 );
	vec3 lightDirNorm	= normalize( lightDir );
	
	float ppDiff		= max( dot( vNormal, lightDirNorm ), 0.0 );
	
	vec3 col			= texture2D( captureTex, pos ).rgb;
	
//	vec3 normalCol	= vNormal * 0.5 + 0.5;
//	
	float shadow		= max( vVertex.z * 0.02 + 0.5, 0.0 );
	
	gl_FragColor.rgb	= vColor.rgb * ppEyeDiff * ppDiff * 1.5 * shadow;
	gl_FragColor.a		= 1.0;
}