#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D basic_texture;

void main()
{
    // TEMP: Keep as solid red until orientation is correct
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);  // Solid red
    vec4 sC = texture(basic_texture, TexCoord);
  
    vec3 finalColor = vec3(sC.b, sC.g, sC.r) * 1.0;
    FragColor = vec4(finalColor, 1.0);
}
