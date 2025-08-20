#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;
in vec3 toLightVector;

uniform sampler2D basic_texture;
uniform sampler2D shadowMap;
uniform vec3 lightDirection;
uniform float ambientStrength = 0.4;  // Slightly higher ambient for first-person weapon
uniform float diffuseStrength = 0.8;  // Strong diffuse for good definition
uniform float specularStrength = 0.2; // Add subtle specular for metallic look

float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal)
{
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Check if fragment is outside light's view
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || 
        projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0; // No shadow outside light frustum
    }
    
    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    // Calculate bias based on normal and light direction
    float bias = max(0.01 * (1.0 - dot(normal, lightDir)), 0.005);
    
    // PCF for softer shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    return shadow;
}

void main()
{
    vec4 color = texture(basic_texture, TexCoord);
    
    // Original Carnivores used face alpha flags, not texture-based alpha testing
    
    // Convert from BGR to RGB (matching original Carnivores format)
    vec3 textureColor = vec3(color.b, color.g, color.r);
    
    // Normalize vectors
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightDirection);
    vec3 viewDir = normalize(-FragPos); // Camera at origin for weapon view
    
    // Ambient lighting
    vec3 ambient = ambientStrength * textureColor;
    
    // Diffuse lighting with proper normal calculation
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * diff * textureColor;
    
    // Specular lighting for metallic weapon look
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // Moderate shininess
    vec3 specular = specularStrength * spec * vec3(1.0); // White specular highlights
    
    // Calculate shadow with normal information
    float shadow = ShadowCalculation(FragPosLightSpace, lightDir, norm);
    
    // Combine lighting with shadow - shadows affect diffuse and specular
    vec3 lighting = ambient + (1.0 - shadow * 0.6) * (diffuse + specular);
    
    // Enhance for old game aesthetic - richer colors and contrast
    lighting = pow(lighting, vec3(0.85)); // Gamma correction for more contrast
    lighting = mix(vec3(dot(lighting, vec3(0.299, 0.587, 0.114))), lighting, 1.3); // Higher saturation\n    \n    // Add subtle color temperature shift for warmer weapon tones\n    lighting.r *= 1.05; // Slightly more red\n    lighting.b *= 0.95; // Slightly less blue
    
    FragColor = vec4(lighting, 1.0);
}
