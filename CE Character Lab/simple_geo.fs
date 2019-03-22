#version 330 core
out vec4 FragColor;

in vec2 texCoord0;

uniform sampler2D basic_texture;

void main()
{
    vec4 sC = texture( basic_texture, texCoord0 );
    float trans = 0.095;
    
    // TODO: Make this conditional
    if (sC.r <= trans && sC.g <= trans && sC.b <= trans) {
        discard;
    }
    
    vec4 lightColor = vec4(sC.b, sC.g, sC.r, 1.f);
    
    FragColor = lightColor;
}
