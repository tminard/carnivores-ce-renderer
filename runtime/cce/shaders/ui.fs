#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D basic_texture;

void main()
{
    // TEMP: Keep as solid red until orientation is correct
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);  // Solid red
    FragColor = texture(basic_texture, TexCoord);
}
