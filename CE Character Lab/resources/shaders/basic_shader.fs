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

    float view_distance = (128.0*256.0);
    float min_distance = view_distance * 0.80;
    float dist_percent = max((gl_FragCoord.z / gl_FragCoord.w) - min_distance, 0.0) / (view_distance - min_distance);
    alpha = 1.0 - clamp(dist_percent, 0.0, 1.0);
    
    if (faceAlpha0 == 0.0 && enable_transparency && sC.r <= trans && sC.g <= trans && sC.b <= trans) {
        discard;
    }

    outputColor = vec4(finalColor, alpha);
}
