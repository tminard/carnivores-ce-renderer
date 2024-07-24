#version 330 core

in vec2 texCoord0;
smooth in float alpha0;
in float EdgeFactor;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform float time; // Time for wave animation
uniform vec4 skyColor;
uniform float view_distance;

const float wave_speed = 0.1;
const float wave_scale = 0.02;

void main()
{
    // ambient and fade
    float min_distance = 256.0 * 6.0; // Start fog
    float max_distance = view_distance;
    float fogFactor = 0.0;
    float distance = gl_FragCoord.z / gl_FragCoord.w;

    vec4 sC = texture(basic_texture, texCoord0);

    if (distance > min_distance) {
        fogFactor = clamp((distance - min_distance) / (max_distance - min_distance), 0.0, 1.0);
        fogFactor = min(fogFactor, 0.65); // Limit the max fill so we keep things visible
    } else {
        // Apply wave distortion to texture coordinates
        vec2 wave = texCoord0 + vec2(sin(texCoord0.x * 10.0 + time * wave_speed) * wave_scale,
                                    cos(texCoord0.y * 10.0 + time * wave_speed) * wave_scale);
        
        sC = texture(basic_texture, wave);
    }

    vec3 finalColor = vec3(sC.b, sC.g, sC.r);
    finalColor = mix(finalColor, skyColor.rgb, fogFactor);
   
    // Ensure the alpha does not go below factor
    float finalAlpha = max(alpha0, 0.15);

    outputColor = mix(vec4(finalColor, finalAlpha), skyColor, 1.0 - EdgeFactor);
}
