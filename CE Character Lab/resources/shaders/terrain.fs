#version 330

in vec2 texCoord0;
in float faceAlpha0;

out vec4 outputColor;

uniform sampler2D basic_texture;

void main()
{
    vec4 sC = texture(basic_texture, texCoord0);

    vec3 finalColor = vec3(sC.b, sC.g, sC.r);
    finalColor = finalColor.rgb;
   
    outputColor = vec4(finalColor, faceAlpha0);
}
