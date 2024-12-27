#version 430


// Input
in vec3 v_position;
in vec3 v_normal;
in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform vec3 generator_position;
uniform float deltaTime;

uniform vec3 curveStart;
uniform vec3 curveControl1;
uniform vec3 curveControl2;
uniform vec3 curveEnd;

uniform vec2 lakeCenter;
uniform float lakeRadius;
uniform float h_max; 

out float vert_lifetime;
out float vert_iLifetime;

struct Particle {
    vec4 position;
    vec4 speed;
    vec4 iposition;
    vec4 ispeed;
    float delay;
    float iDelay;
    float lifetime;
    float iLifetime;
};

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 bezierInterpolate(float t, vec3 p0, vec3 p1, vec3 p2, vec3 p3) {
    float u = 1.0 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    vec3 p = uuu * p0; 
    p += 3.0 * uu * t * p1; 
    p += 3.0 * u * tt * p2; 
    p += ttt * p3; 

    return p;
}

float ConcaveConvexDisplacement(float x, float z, vec2 center, float radius, float h_max) {
    float d = distance(vec2(x, z), center) / radius;
    if (d < 1.0f) {
        return (d * d / 2.0) * h_max; 
    } else {
        return (1.0 - ((2.0 - d) * (2.0 - d)) / 2.0) * h_max; 
    }
}

layout(std430, binding = 0) buffer particles {
    Particle data[];
};

void main() {
    vec3 pos = data[gl_VertexID].position.xyz;
    vec3 spd = data[gl_VertexID].speed.xyz;
    float delay = data[gl_VertexID].delay;

    delay -= deltaTime;

    if (delay > 0) {
        data[gl_VertexID].delay = delay;
        gl_Position = Model * vec4(generator_position, 1);
        return;
    }


    float displacement = ConcaveConvexDisplacement(pos.x, pos.z, lakeCenter, lakeRadius, h_max);
    pos.y = displacement; 

    float distToLake = distance(vec2(pos.x, pos.z), lakeCenter);
    float lakeFactor = clamp(distToLake / lakeRadius, 0.0f, 1.0f);

    float distanceFromCenter = sqrt(pos.x * pos.x + pos.z * pos.z);
    float depthFactor = clamp(distanceFromCenter / lakeRadius, 0.0f, 1.0f); 
    float convexShape = smoothstep(0.0f, 2.0f, depthFactor);                

    float lakeBasin = h_max * (1.0 - convexShape) * (1.0 - convexShape) * (-0.6); 

    float mountainNoise = h_max * 0.2 * (
        sin(pos.x * 0.3) * cos(pos.z * 0.6) +
        sin(pos.z * 0.5) * cos(pos.x * 0.8) +
        sin(pos.x * 0.2) * cos(pos.z * 0.4)
    );

    mountainNoise *= convexShape;
    pos.y += lakeBasin + mountainNoise + 8;

    float t = mod(data[gl_VertexID].lifetime / data[gl_VertexID].iLifetime, 1.0);
    vec3 bezierPos = bezierInterpolate(t, curveStart, curveControl1, curveControl2, curveEnd);

    pos -= vec3(pos.x, bezierPos.y, pos.z) * deltaTime;
    pos += spd * deltaTime;

    spd -= vec3(0, 0.9, 0);

    if (pos.y < 0) {
        pos = data[gl_VertexID].iposition.xyz + generator_position;
        spd = data[gl_VertexID].ispeed.xyz;
        delay = data[gl_VertexID].iDelay;
    }

    data[gl_VertexID].position.xyz = pos;
    data[gl_VertexID].speed.xyz = spd;
    data[gl_VertexID].delay = delay;

    gl_Position = Model * vec4(pos, 1);
}
