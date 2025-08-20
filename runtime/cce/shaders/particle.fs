#version 330 core
in vec2 TexCoord;
in vec3 ParticleColor;
in float Alpha;

out vec4 FragColor;

uniform sampler2D particleTexture;
uniform float alphaMultiplier;

void main()
{
    vec4 texColor = texture(particleTexture, TexCoord);
    FragColor = vec4(ParticleColor * texColor.rgb, texColor.a * Alpha * alphaMultiplier);
    
    // Discard fully transparent pixels
    if (FragColor.a < 0.01) discard;
}