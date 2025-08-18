#version 330

in highp vec2 texCoord0;
in highp vec2 texCoord1;
in vec3 surfaceNormal;
in vec3 toLightVector;
in vec2 quadCoord;
in float wetness;
in highp vec2 out_textCoord_clouds;
in vec3 FragPos;
in vec4 FragPosLightSpace;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform sampler2D skyTexture;
uniform float view_distance;
uniform vec4 distanceColor;

uniform float ambientStrength = 0.25;
uniform float diffuseStrength = 0.75;
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
    float bias = 0.001;
    
    // Enhanced PCF for smoother shadows (larger sampling area)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    int sampleRadius = 2; // Larger radius for smoother shadows
    int sampleCount = 0;
    
    for(int x = -sampleRadius; x <= sampleRadius; ++x)
    {
        for(int y = -sampleRadius; y <= sampleRadius; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            sampleCount++;
        }
    }
    shadow /= float(sampleCount);
    
    return shadow;
}

void main()
{
    vec4 sC;

    // Determine which texture coordinates to use for the current face
    if (mod(gl_PrimitiveID, 2) == 0) {
        sC = texture(basic_texture, texCoord1);
    } else {
        sC = texture(basic_texture, texCoord0);
    }

    // Ambient and fade
    float min_distance = view_distance * 0.50; // Start fog at half distance
    float max_distance = view_distance;
    float fogFactor = 0.0;
    float distance = gl_FragCoord.z / gl_FragCoord.w;

    if (distance > min_distance) {
        fogFactor = clamp((distance - min_distance) / (max_distance - min_distance), 0.0, 1.0);
        fogFactor = min(fogFactor, 0.45); // Limit the max fill so we keep things visible
    }

    // Real-time lighting (same as world objects)
    vec3 norm = normalize(surfaceNormal);
    vec3 lightDir = normalize(-lightDirection); // Use the world object light direction
    
    // Convert texture from BGR to RGB format
    vec3 textureColor = vec3(sC.b, sC.g, sC.r);
    
    // Ambient lighting
    vec3 ambient = ambientStrength * textureColor;
    
    // Diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * diff * textureColor;
    
    // Combine ambient and diffuse
    vec3 finalColor = ambient + diffuse;
    
    // Apply shadows if enabled
    if (enableShadows) {
        float shadow = ShadowCalculation(FragPosLightSpace);
        
        // Apply shadow to the final color - reduce brightness where shadows are cast
        finalColor = finalColor * (1.0 - shadow * 0.3); // 30% shadow intensity
    }

    finalColor = mix(finalColor, distanceColor.rgb, fogFactor);

    // Apply a wetness effect
    // if (wetness > 0.0) {
    //     finalColor = mix(finalColor, distanceColor.rgb, wetness * 0.10);
    //     // alpha *= mix(1.0, 0.5, wetness); // Adjust alpha for underwater visibility
    // }

    outputColor = vec4(finalColor, 1.0);
}
