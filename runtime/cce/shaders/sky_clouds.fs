#version 330 core
out vec4 outputColor;

in vec2 TexCoords0;
in vec3 WorldPos;

uniform sampler2D basic_texture;
uniform vec4 sky_color;

void main()
{
    // Calculate circular distance from center of cloud plane
    float distFromCenter = length(WorldPos.xz);
    
    // Create circular fade - start fading at 2500 units, fully fade by 3500 units
    float fadeStart = 2500.0;
    float fadeEnd = 3500.0;
    float fadeAmount = clamp((distFromCenter - fadeStart) / (fadeEnd - fadeStart), 0.0, 1.0);
    
    vec4 sC = texture(basic_texture, TexCoords0);
    vec3 cloudColor = vec3(sC.b, sC.g, sC.r); // BGR to RGB conversion
    
    // Blend cloud color with sky color based on circular distance
    vec3 finalColor = mix(cloudColor, sky_color.rgb, fadeAmount);

    outputColor = vec4(finalColor, 1.0);
}
