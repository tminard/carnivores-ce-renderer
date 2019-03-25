#version 330 core
out vec4 FragColor;

in vec2 texCoord0;

uniform sampler2D basic_texture;

void main()
{
    vec4 tx = texture( basic_texture, texCoord0 );
    vec3 textureColor = vec3(tx.b, tx.g, tx.r);
    float trans = 0.095;
    float dist = (gl_FragCoord.z / gl_FragCoord.w);

    if (dist < 256 * 50)
    {
        discard;
    }
    
    if (tx.r <= trans && tx.g <= trans && tx.b <= trans) {
        discard;
    }

    FragColor = vec4(textureColor, 1.0);
}
