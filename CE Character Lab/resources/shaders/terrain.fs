#version 330

in vec2 texCoord0;
in float brightness0;

out vec4 outputColor;

uniform sampler2D basic_texture;

void main()
{
    vec4 sC = texture(basic_texture, texCoord0, -4);
    float percent = 1.0; //(55.0 - brightness0) / 55.0;

    vec3 finalColor = vec3(sC.b * percent, sC.g * percent, sC.r * percent);
    finalColor = finalColor.rgb;
   
    outputColor = vec4(finalColor, 1.0);
}
