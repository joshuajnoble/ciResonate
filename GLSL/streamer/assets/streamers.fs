#version 120

uniform sampler2D tex;

uniform vec4 WireColor;
uniform vec4 EmptyColor;
uniform float Balance;

varying vec3 TexCoord;

float numeric_stripe(float Value, float Balance, float Oversample, float PatternScale)
{
    float width = abs(dFdx(Value)) + abs(dFdy(Value));
    float w = width*Oversample;
    float x0 = Value/PatternScale - (w/2.0);
    float x1 = x0 + w;
    float i0 = (1.0-Balance)*floor(x0) + max(0.0, fract(x0)-Balance);
    float i1 = (1.0-Balance)*floor(x1) + max(0.0, fract(x1)-Balance);
    float stripe = (i1 - i0)/w;
    stripe = min(1.0,max(0.0,stripe)); 
    return stripe;
}

void main()
{

	
    float stripex = numeric_stripe(TexCoord.x, 1.-Balance, 1., 0.2);
    float stripey = numeric_stripe(TexCoord.y, 1.-Balance, 1., 0.2);
    float stripez = numeric_stripe(TexCoord.z, 1.-Balance, 1., 0.2);
    
    float check = stripex * stripey;// * stripez;
    
    vec4 empty = texture2D(tex, TexCoord.xy);
    
	gl_FragColor = mix(WireColor,empty,check);;
}
