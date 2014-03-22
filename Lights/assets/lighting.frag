#version 120

#define MAX_LIGHTS 8

struct material
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

// note: these *should* be passed in via uniform buffer objects
// but it isn't supported in GLES2.0

struct light
{
    vec4 ambient;
    float type; // 0 = pointlight 1 = directionlight
    vec4 position; // where are we
    vec4 diffuse; // how diffuse
    vec4 specular; // what kinda specular stuff we got going on?
    // attenuation?
    float constantAttenuation, linearAttenuation, quadraticAttenuation;
    // only for spot
    //float spotCutoff, spotExponent;
    //vec3 spotDirection;
    vec3 halfVector; // only for dir
};

uniform sampler2D tex0;

//uniform material currentMaterial;
uniform vec4 mat_ambient;
uniform vec4 mat_diffuse;
uniform vec4 mat_specular;
uniform float mat_shininess;

// this should be generated?
uniform int numberOfLights;

varying vec4 outColor; // this is the ultimate color for this vertex
varying vec2 outtexcoord; // pass the texCoord if needed
varying vec3 transformedNormal;
//in mat4 currentMaterial; // this is all our material data

////////////////////////////////////////////////////////////
// Eye-coordinate position of vertex
varying vec4 eyePosition;
varying vec3 eyePosition3;
varying vec3 eye;

// these are passed in from OF programmable renderer
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;

uniform light lights[MAX_LIGHTS];

vec4 Ambient;
vec4 Diffuse;
vec4 Specular;

mat3x4 pointLight( int i, vec3 normal, vec3 eye, vec3 ecPosition3 )
{
    float nDotVP;       // normal . light direction
    float nDotHV;       // normal . light half vector
    float pf;           // power factor
    float attenuation;  // computed attenuation factor
    float d;            // distance from surface to light source
    vec3  VP;           // direction from surface to light position
    vec3  halfVector;   // direction of maximum highlights
    
    // Compute vector from surface to light position
    VP = vec3 (lights[i].position.xyz) - ecPosition3;
    
    // Compute distance between surface and light position
    d = length(VP);
    
    // Normalize the vector from surface to light position
    VP = normalize(VP);
    
    // Compute attenuation
    attenuation = 1.0 / (lights[i].constantAttenuation + lights[i].linearAttenuation * d + lights[i].quadraticAttenuation * d * d);
    
    halfVector = normalize(VP + eye);
    
    nDotVP = max(0.0, dot(normal, VP));
    nDotHV = max(0.0, dot(normal, halfVector));
    
    // ha! no branching :)
    pf = mix(0.0, pow(nDotHV, mat_shininess), step(0.0000001, nDotVP));
    
    //   Ambient  += lights[i].ambient * attenuation;
    //   Diffuse  += lights[i].diffuse * nDotVP * attenuation;
    //   Specular += lights[i].specular * pf * attenuation;
    
    mat3x4 lightResult = mat3x4(lights[i].ambient * attenuation, lights[i].diffuse * nDotVP * attenuation, lights[i].specular * pf * attenuation);
    return lightResult;
}

mat3x4 directionalLight(int i, vec3 normal)
{
    float nDotVP;         // normal . light direction
    float nDotHV;         // normal . light half vector
    float pf;             // power factor
    
    nDotVP = max(0.0, dot(normal, normalize(vec3 (lights[i].position))));
    nDotHV = max(0.0, dot(normal, vec3 (lights[i].halfVector)));
    
    pf = mix(0.0, pow(nDotHV, mat_shininess), step(0.0000001, nDotVP));
    
    mat3x4 lightResult = mat3x4(lights[i].ambient, (lights[i].diffuse * nDotVP), lights[i].specular * pf);
    return lightResult;
}

//////////////////////////////////////////////////////
// here's the main method
//////////////////////////////////////////////////////


void main (void)
{

    vec4 localColor;

    Ambient = vec4(0., 0., 0., 0.);
    Diffuse = vec4(0., 0., 0., 0.);
    Specular = vec4(0., 0., 0., 0.);
    
    ////////////////////////////////////////////////////////////
    // here's the loop over each light
    // the count will be generated in the shader, em,
    for( int i = 0; i < 2; i++ )
    {
        
        mat3x4 pointCalc = pointLight(i, transformedNormal, eye, eyePosition3);
        mat3x4 dirCalc = directionalLight(i, transformedNormal);
        
        vec4 amb = mix(pointCalc[0].xyzw, dirCalc[0].xyzw, step(1., lights[i].type));
        vec4 dif = mix(pointCalc[1].xyzw, dirCalc[1].xyzw, step(1., lights[i].type));
        vec4 spec = mix(pointCalc[2].xyzw, dirCalc[2].xyzw, step(1., lights[i].type));
        
        Ambient += amb;
        Diffuse += dif;
        Specular += spec;
        
        //Ambient += pointCalc[0];
        //Diffuse += pointCalc[1];
        //Specular += pointCalc[2];
        
    }

    ////////////////////////////////////////////////////////////
    // now add the material info
    
    localColor = Ambient * max(mat_ambient, 0.00001) + Diffuse  * max(mat_diffuse, 0.00001) + texture2D(tex0, outtexcoord);
    localColor += Specular * max(mat_specular, 0.00001);

    
    ////////////////////////////////////////////////////////////
    // now get the color ready
    gl_FragColor = clamp( localColor, 0.0, 1.0 );
    //gl_FragColor = vec4( 1, 0, 0, 1 );
    
    
}

