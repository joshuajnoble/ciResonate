uniform vec2 resolution;
uniform float time;

// replace this
uniform sampler2D tex0;

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(void)
{
    vec2 q = gl_FragCoord.xy / resolution.xy;
    
    // subtle zoom in/out
    vec2 uv = 0.5 + (q-0.5)*(0.98 + 0.006*sin(0.9*time));
    
    vec3 oricol = texture2D(tex0,vec2(q.x,1.0-q.y)).xyz;
    vec3 col;
    
    // start with the source texture and misalign the rays it a bit
    // TODO animate misalignment upon hit or similar event
    col.r = texture2D(tex0,vec2(uv.x+0.003,-uv.y)).x;
    col.g = texture2D(tex0,vec2(uv.x+0.000,-uv.y)).y;
    col.b = texture2D(tex0,vec2(uv.x-0.003,-uv.y)).z;
    
    // contrast curve
    col = clamp(col*0.5+0.5*col*col*1.2,0.0,1.0);
    
    //vignette
    col *= 0.6 + 0.4*16.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y);
    
    //color tint
    col *= vec3(0.9,1.0,0.7);
    
    //scanline (last 2 constants are crawl speed and size)
    //TODO make size dependent on viewport
    col *= 0.8+0.2*sin(10.0*time+uv.y*900.0);
    
    //flickering (semi-randomized)
    col *= 1.0-0.07*rand(vec2(time, tan(time)));
    
    //smoothen
    float comp = smoothstep( 0.2, 0.7, sin(time) );
    col = mix( col, oricol, clamp(-2.0+2.0*q.x+3.0*comp,0.0,1.0) );
    
    gl_FragColor = vec4(col,1.0);
}