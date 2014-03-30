/ -- Lighting varyings (from Vertex Shader)
varying vec3 norm, lightDir0, halfVector0;
varying vec4 diffuse0, ambient;
 
vec4 phongDir_FS()
{
    vec3 halfV;
    float NdotL, NdotHV;
     
    // The ambient term will always be present
    vec4 color = ambient;
     
    // compute the dot product between normal and ldir
    NdotL = max(dot(norm, lightDir0),0.0);
     
    if (NdotL > 0.0) {
        color += diffuse0 * NdotL;
        halfV = normalize(halfVector0);
        NdotHV = max(dot(norm, halfV), 0.0);
        color +=    gl_FrontMaterial.specular * 
                gl_LightSource[0].specular * 
                pow(NdotHV, gl_FrontMaterial.shininess);
    }   
    return color;
}
 
///////////////
// Main Loop //
///////////////
 
void main()
{
    // Call lighting function and return result
    gl_FragColor = phongDir_FS();
}