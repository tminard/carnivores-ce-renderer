#version 330

in vec2 texCoord0;
in vec2 texCoord1;
smooth in float brightness0;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform float view_distance;

void main()
{
    vec4 sC;

    if (mod(gl_PrimitiveID, 2) == 0) {
        sC = texture(basic_texture, texCoord1, -2);
    } else {
        sC = texture(basic_texture, texCoord0, -2);
    }

    float percent = (brightness0 / 255.0);
    float min_distance = view_distance * 0.80;
    float alpha = 1.0;
    float dist_percent = max((gl_FragCoord.z / gl_FragCoord.w) - min_distance, 0.0) / (view_distance - min_distance);
    alpha = 1.0 - clamp(dist_percent, 0.0, 1.0);

    vec3 finalColor = vec3(sC.b * percent, sC.g * percent, sC.r * percent);
    finalColor = finalColor.rgb;
   
    outputColor = vec4(finalColor, alpha);
}
