#version 430

// Input
layout(location = 0) in vec2 texture_coord;

// Uniform properties
uniform sampler2D texture_1;

// Output
layout(location = 2) out vec4 out_color;


void main()
{
    vec4 texColor = texture(texture_1, texture_coord);
    if (texColor.a == 1)
        discard;
    out_color = vec4(texColor.rgb, 1.0);

    
}
