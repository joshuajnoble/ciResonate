#version 120

// The higher the value, the bigger the contrast between the fur length.
#define FUR_STRENGTH_CONTRAST 2.0

// The higher the value, the less fur.
#define FUR_STRENGTH_CAP 0.3

uniform sampler2D u_textureFurColor;

uniform sampler2D u_textureFurStrength;

uniform vec3 u_lightDirection;

varying vec3 v_normal;
varying vec2 v_texCoord;

varying float v_furStrength;


float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
	// Note: All calculations are in camera space.

	vec3 normal = normalize(v_normal);

	// Orthogonal fur to light is still illumintated. So shift by one, that only fur targeting away from the light do get darkened. 
	float intensity = clamp(dot(normal, u_lightDirection) + 1.0, 0.0, 1.0);
	
	//float furStrength = clamp(v_furStrength * texture2D(u_textureFurStrength, v_texCoord).r * FUR_STRENGTH_CONTRAST - FUR_STRENGTH_CAP, 0.0, 1.0);
	float furStrength = clamp(v_furStrength * rand(v_texCoord) * FUR_STRENGTH_CONTRAST - FUR_STRENGTH_CAP, 0.0, 1.0);

	gl_FragColor = vec4(texture2D(u_textureFurColor, v_texCoord).rgb * intensity, furStrength);
}   