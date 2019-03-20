#version 330

in vec2 texCoord0;
in float distanceToCamera;

out vec4 outputColor;

uniform sampler2D basic_texture;

const vec3 fogColor = vec3(0.94, 0.97, 1.0);
const int FOG_START = 10240;
const int FOG_END = 12544;

void main()
{
    vec4 sC = texture( basic_texture, texCoord0 );
    float dist = abs(distanceToCamera);
    float trans = 0.095;
    
    // TODO: Make this conditional
    if (sC.r <= trans && sC.g <= trans && sC.b <= trans) {
        discard;
    }
    
    float fogFactor = (FOG_END - dist)/(FOG_END - FOG_START);
    vec3 finalColor;
    vec3 lightColor = vec3(sC.b, sC.g, sC.r);
    
    lightColor = 1.55 * lightColor.rgb * vec3(0.94, 0.97, 1.0);
    fogFactor = clamp(fogFactor, 0.0, 1.0 );
    
    finalColor = mix(fogColor, lightColor, fogFactor);
    outputColor = vec4(finalColor, 1.0f);
    
    //outputColor = vec4(sC.b, sC.g, sC.r, 1.0f);
}
