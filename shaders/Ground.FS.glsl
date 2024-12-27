#version 330 core

in vec2 texture_coord;
in vec3 world_position;
in vec3 world_normal;

uniform sampler2D u_texture_0;
uniform float clip_plane_y;

// Outputs
layout(location = 0) out vec4 out_world_position;
layout(location = 1) out vec4 out_world_normal;
layout(location = 2) out vec4 out_color;

void main()
{
    if (world_position.y < clip_plane_y) {
        discard;
    }
 
    // Output
    out_world_position = vec4(world_position, 1.0);
    out_world_normal = vec4(normalize(world_normal), 1.0);
    out_color = texture(u_texture_0, texture_coord); 
}
