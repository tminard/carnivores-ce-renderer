#version 330 core
out vec4 outputColor;

in vec2 TexCoords0;

uniform sampler2D basic_texture;
uniform vec4 sky_color;

void main()
{
    vec4 sC = texture( basic_texture, TexCoords0 );
    vec4 finalColor = vec4(sC.b, sC.g, sC.r, 1.0);

    outputColor = finalColor;
}

