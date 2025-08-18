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

// New uniforms for impact marker coloring
uniform bool useCustomColor = false;
uniform vec3 customColor = vec3(1.0, 0.0, 0.0);

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
    // Check if we should use custom color (for impact markers)
    if (useCustomColor) {
        FragColor = vec4(customColor, 1.0);
        return;
    }
    
    // Use texture for normal rendering
    vec4 texColor = texture(basic_texture, texCoord0);
    
    // Apply basic lighting
    vec3 lightDir = normalize(-lightDirection);
    
    // Simple ambient + diffuse lighting
    float ambient = ambientStrength;
    float diffuse = max(dot(vec3(0, 0, 1), lightDir), 0.0) * diffuseStrength; // Assume normal pointing up for simple geo
    
    float lightFactor = ambient + diffuse;
    
    // Apply shadow if enabled
    float shadow = 0.0;
    if (enableShadows) {
        shadow = ShadowCalculation(FragPosLightSpace);
    }
    
    vec3 finalColor = texColor.rgb * lightFactor * (1.0 - shadow * 0.5);
    FragColor = vec4(finalColor, texColor.a);
}
