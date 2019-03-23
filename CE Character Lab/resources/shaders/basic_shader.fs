#version 330

in vec2 texCoord0;
in float faceAlpha0;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform bool enable_transparency;

void main()
{
    vec4 sC = texture( basic_texture, texCoord0 );
    vec3 finalColor = vec3(sC.b, sC.g, sC.r);
    float alpha = 1.0;
    float trans = 0.095;
    
    if (faceAlpha0 < 0.5 && enable_transparency && sC.r <= trans && sC.g <= trans && sC.b <= trans) {
        discard;
    }

    outputColor = vec4(finalColor, alpha);
}
