#version 330 core

in vec3 FragPos;
in vec2 TexCoord;
in vec4 FragPosLightSpace;
in vec3 toLightVector;

out vec4 FragColor;

uniform sampler2D basic_texture;
uniform sampler2D shadowMap;
uniform vec3 lightDirection;
uniform float ambientStrength = 0.3;
uniform float diffuseStrength = 0.7;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir)
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
    
    // Calculate bias to prevent shadow acne (simplified since we don't have normals)
    float bias = 0.005;
    
    // Simple shadow test with small PCF for softer edges
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
    
    // Check for transparency (matching simple_geo.fs logic)
    float trans = 0.095;
    if (color.r <= trans && color.g <= trans && color.b <= trans) {
        discard;
    }
    
    // Convert from BGR to RGB (matching simple_geo.fs)
    vec3 textureColor = vec3(color.b, color.g, color.r);
    
    // Simple lighting calculation (no normals available)
    vec3 lightDir = normalize(-lightDirection);
    
    // Ambient lighting
    vec3 ambient = ambientStrength * textureColor;
    
    // Simple directional diffuse without normals - just fade with light direction
    vec3 diffuse = diffuseStrength * textureColor;
    
    // Calculate shadow
    float shadow = ShadowCalculation(FragPosLightSpace, lightDir);
    
    // Combine lighting with shadow - shadows darken the diffuse component
    vec3 lighting = ambient + (1.0 - shadow * 0.7) * diffuse;
    
    FragColor = vec4(lighting, 1.0);
}