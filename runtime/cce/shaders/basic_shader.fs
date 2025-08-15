#version 330

in vec2 texCoord0;
in float faceAlpha0;
in vec3 surfaceNormal;
in vec3 toLightVector;
in vec3 FragPos;
in vec4 FragPosLightSpace;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform bool enable_transparency;
uniform float view_distance;
uniform vec4 distanceColor;

uniform float ambientStrength = 0.65;
uniform float diffuseStrength = 0.35;
uniform float time;
uniform sampler2D shadowMap;
uniform vec3 lightDirection = vec3(0.5, -1.0, 0.5);
uniform bool enableShadows = false;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    if (!enableShadows) return 0.0;
    
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Check if fragment is outside light's view
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || 
        projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0;
    }
    
    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    // Calculate bias to prevent shadow acne
    float bias = 0.002;
    
    // Simple shadow test with PCF for softer edges
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
    vec4 sC = texture(basic_texture, texCoord0);

    // Transparency discard (do this first)
    float trans = 0.095;
    if (faceAlpha0 == 0.0 && enable_transparency && sC.r <= trans && sC.g <= trans && sC.b <= trans) {
        discard;
    }

    // Lighting calculations
    vec3 unitSurfaceNormal = normalize(surfaceNormal);
    vec3 unitToLightVector = normalize(toLightVector);

    float diffuse = max(dot(unitSurfaceNormal, unitToLightVector), 0.0);
    float brightness = ambientStrength + diffuse * diffuseStrength;

    // Apply the lighting to the texture color
    vec3 finalColor = vec3(sC.b, sC.g, sC.r) * brightness;
    
    // Apply shadows if enabled
    if (enableShadows) {
        float shadow = ShadowCalculation(FragPosLightSpace);
        finalColor = finalColor * (1.0 - shadow * 0.4); // Natural shadow darkening
    }

    // Fog effect
    float min_distance = view_distance * 0.50;
    float max_distance = view_distance;
    float fogFactor = 0.0;
    float distance = gl_FragCoord.z / gl_FragCoord.w;

    if (distance > min_distance) {
        fogFactor = clamp((distance - min_distance) / (max_distance - min_distance), 0.0, 1.0);
        fogFactor = min(fogFactor, 0.45);
    }

    finalColor = mix(finalColor, distanceColor.rgb, fogFactor);
    outputColor = vec4(finalColor, 1.0);
}
