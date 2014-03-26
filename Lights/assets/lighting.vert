#version 120

varying vec4 outColor; // this is the ultimate color for this vertex
varying vec2 outtexcoord; // pass the texCoord if needed

varying vec3 transformedNormal;

////////////////////////////////////////////////////////////
// Eye-coordinate position of vertex
varying vec4 eyePosition;
varying vec3 eyePosition3;
varying vec3 eye;

// not using these, glsl1.2
//uniform mat4 modelViewMatrix;
//uniform mat4 projectionMatrix;
//uniform mat4 textureMatrix;
//uniform mat4 modelViewProjectionMatrix;
//attribute vec4 position;
//attribute vec4 color;
//attribute vec4 normal;
//attribute vec2 texcoord;


void main (void)
{
    float alphaFade = 1.0;
    ////////////////////////////////////////////////////////////
    // Eye-coordinate position of vertex
    vec4 eyePosition = (gl_ModelViewMatrix * gl_Vertex) * -1;
    transformedNormal = normalize(gl_NormalMatrix * gl_Normal);

    ////////////////////////////////////////////////////////////
    // let's do some eye positioning
    eyePosition3 = (eyePosition.xyz) / eyePosition.w;
    eye = vec3 (0.0, 0.0, 1.0);

    outtexcoord = (gl_TextureMatrix[0] * vec4(gl_MultiTexCoord0.x, gl_MultiTexCoord0.y,0,1)).xy;
    ////////////////////////////////////////////////////////////
    // Do fixed functionality vertex transform
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
}