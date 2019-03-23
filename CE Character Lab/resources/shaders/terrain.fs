#version 330

in vec2 texCoord0;

out vec4 outputColor;

uniform sampler2D basic_texture;

void main()
{
    vec4 sC = texture(basic_texture, texCoord0);

    vec3 finalColor = vec3(sC.b, sC.g, sC.r);
    finalColor = 1.55 * finalColor.rgb * vec3(0.94, 0.97, 1.0);
   
    outputColor = vec4(finalColor, 1.0);
}
