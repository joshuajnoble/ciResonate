#version 120

varying vec3 fcolor;

void main() {
    gl_FragColor = vec4(fcolor, 1.0);
    //gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}