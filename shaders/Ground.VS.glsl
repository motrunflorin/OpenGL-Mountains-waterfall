#version 330 core

layout(location = 0) in vec3 v_position;     
layout(location = 1) in vec3 v_normal;         
layout(location = 2) in vec2 v_texture_coord;  

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec2 texture_coord;    
out vec3 world_normal;    
out vec3 world_position;   

void main()
{
    texture_coord = v_texture_coord;

    world_position = (Model * vec4(v_position, 1.0)).xyz;
    world_normal = mat3(transpose(inverse(Model))) * v_normal; 

    gl_Position = Projection * View * vec4(world_position, 1.0);
}
