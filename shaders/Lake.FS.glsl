#version 330 core
in vec2 texture_coord;       
in vec3 world_position;      
in vec3 world_normal;        

uniform sampler2D reflection_texture; 
uniform float clip_plane_y;            

layout(location = 0) out vec4 out_world_position;
layout(location = 1) out vec4 out_world_normal;
layout(location = 2) out vec4 out_color;

void main() {

    if (world_position.y < clip_plane_y) {
        discard;
    }

    out_world_position = vec4(world_position, 1.0);
    out_world_normal = vec4(normalize(world_normal), 1.0);

    ivec2 coord = ivec2(gl_FragCoord.xy);
    coord.y = 1024 - coord.y;

    vec4 reflectionColor = texelFetch(reflection_texture, coord, 0);

    out_color = reflectionColor;
}
