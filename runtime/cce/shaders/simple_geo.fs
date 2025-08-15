#version 330 core
out vec4 FragColor;

in vec2 texCoord0;
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform sampler2D basic_texture;
uniform sampler2D shadowMap;
uniform vec3 lightDirection = vec3(0.5, -1.0, 0.5);
uniform bool enableShadows = false;
uniform float ambientStrength = 0.3;
uniform float diffuseStrength = 0.7;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    if (!enableShadows) return 0.0;
    
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // DEBUG: For now, just return a test pattern based on screen position
    // This will help us see if shadow calculation is running at all
    float testShadow = step(0.5, fract(gl_FragCoord.x * 0.01)) * step(0.5, fract(gl_FragCoord.y * 0.01));
    if (testShadow > 0.5) return 1.0;
    
    return 0.0;
}

void main()
{
    // FORCE show debug colors to test shader execution
    // All objects should appear in these debug colors
    
    if (enableShadows) {
        // Shadow-enabled objects = BRIGHT GREEN
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);  
    } else {
        // Regular objects = BRIGHT RED
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
}
