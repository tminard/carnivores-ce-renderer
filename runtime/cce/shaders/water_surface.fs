#version 330 core

in vec2 texCoord0;
smooth in float alpha0;
in float EdgeFactor;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform float time; // Time for wave animation
uniform vec4 skyColor;

const float wave_speed = 0.1;
const float wave_scale = 0.02;

void main()
{
    // Apply wave distortion to texture coordinates
    vec2 wave = texCoord0 + vec2(sin(texCoord0.x * 10.0 + time * wave_speed) * wave_scale,
                                 cos(texCoord0.y * 10.0 + time * wave_speed) * wave_scale);
    
    vec4 sC = texture(basic_texture, wave);

    vec3 finalColor = vec3(sC.b, sC.g, sC.r);
   
    // Ensure the alpha does not go below factor
    float finalAlpha = max(alpha0, 0.15);

    outputColor = mix(vec4(finalColor, finalAlpha), skyColor, 1.0 - EdgeFactor);
    //outputColor = vec4(finalColor, finalAlpha);
}
