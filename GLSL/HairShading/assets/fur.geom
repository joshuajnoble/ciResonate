#version 120
#extension GL_EXT_geometry_shader4 : enable
#extension GL_EXT_gpu_shader4 : enable

#define N 12

//layout(triangles) in;
//layout(line_strip) out;
//layout(max_vertices = N) out;

const float NSTEPS = N - 1.0;

//uniform float time_stop;
uniform float hairLength = 16.0;
uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;

varying out vec3 fcolor;

void main() {
    vec3 a = vec3(gl_PositionIn[1] - gl_PositionIn[0]);
    vec3 b = vec3(gl_PositionIn[2] - gl_PositionIn[0]);
    
    vec3 center = vec3(gl_PositionIn[0] + gl_PositionIn[1] + gl_PositionIn[2]) / 3.0;
    vec3 normal = normalize(cross(b, a));
    
    fcolor = vec3(0.0);
    gl_Position = u_projectionMatrix * u_modelViewMatrix * vec4(center, 1.0);
    EmitVertex();
    
    // x(t) = normal.x * t + center.x;
    // y(t) = -4.9 * t * t + normal.y * t + center.y;
    // z(t) = normal.z * t + center.z;
    
    float delta = hairLength / float(NSTEPS);
    float t = delta;
    for (int i = 0; i < NSTEPS; ++i) {
        fcolor = vec3(t / hairLength);
        vec3 position = normal * t + center;
        position.y += 0.1 * t * t;
        gl_Position = u_projectionMatrix * u_modelViewMatrix * vec4(position, 1.0);
        EmitVertex();
        t += delta;
    }
    EndPrimitive();
}