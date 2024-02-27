#version 460 core
out vec4 FragColor;

uniform vec2 resolution;
uniform float time;

// lower-left pixel 0,0
layout(pixel_center_integer) in vec4 gl_FragCoord;

float map(vec3 p) {

    float d = 0, aux_d;

    // Sphere
    vec3 sphere_center = vec3(0);
    sphere_center.x += sin(time + 2*time);
    sphere_center.y += sin(time + 3*time);
    float sphere_radius = abs(sin(time) + 0.5);
    d = length(p - sphere_center) - sphere_radius;
    
    // Floor
    vec3 plane_normal = vec3(0,1,0);
    vec3 plane_point = vec3(0,-1,0);
    aux_d = dot(p - plane_point, plane_normal);

    d = min(d, aux_d);

    return d;
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    vec2 screen_center = vec2(0.5);
    float aspect_ratio = resolution.x / resolution.y;

    // Fix aspect ratio
    uv.x *= aspect_ratio;
    screen_center.x *= aspect_ratio;

    // Init ray marching
    vec3 ray_origin = vec3(0,0,-3);
    vec3 ray_direction = normalize(vec3(uv - screen_center, 1));
    float distance_traveled = 0;

    for(int i = 0; i < 40; i++) {

        // Signed distance function
        float d = map(ray_origin);

        // Object hit
        if (d < 0.01) break;

        // March ray
        distance_traveled += d;
        ray_origin += d * ray_direction;
    }

    // Color
    distance_traveled *= 0.1;
    FragColor = vec4(distance_traveled.xxx, 1.0);
}
