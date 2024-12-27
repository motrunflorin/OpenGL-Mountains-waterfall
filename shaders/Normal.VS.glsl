#version 330 core

// Input
in vec3 text_coord;  


uniform mat4 View;       
uniform mat4 Projection; 

out vec3 text_coords;

void main()
{
    text_coords = text_coord; 

   vec4 pos = Projection * View * vec4(text_coord, 1.0);

   gl_Position = pos.xyww;
    
    
}