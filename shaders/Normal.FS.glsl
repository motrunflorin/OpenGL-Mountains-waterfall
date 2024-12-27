#version 330 core

// Input
in vec3 text_coords;

// Uniform properties
uniform samplerCube texture_cubemap;

// Output
layout(location = 2) out vec4 out_color;


void main()
{
     out_color = texture(texture_cubemap, text_coords);

}