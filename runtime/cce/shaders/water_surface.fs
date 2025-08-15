#version 330 core

in vec2 texCoord0;
smooth in float alpha0;
in float EdgeFactor;
in vec3 surfaceNormal;
in vec3 toLightVector;
in vec2 cloudTexCoord;
in vec3 viewDirection;
in vec2 heightmapCoord;
flat in uint isDangerWater;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform sampler2D skyTexture;
uniform sampler2D heightmapTexture;
uniform float time; // Time for wave animation
uniform vec4 skyColor;
uniform float view_distance;

uniform vec3 cameraPos;
uniform float waterLevel;
uniform float heightmapScale;

uniform float ambientStrength = 0.45;
uniform float diffuseStrength = 0.55;

const float wave_speed = 0.1;
const float wave_scale = 0.02;

void main()
{
    // Calculate water depth first for use throughout the shader
    float terrainHeight = texture(heightmapTexture, heightmapCoord).r;
    float waterDepth = waterLevel - terrainHeight;
    
    // Handle negative depths (water below terrain) by using absolute value or clamping
    if (waterDepth < 0.0) {
        waterDepth = 0.0; // Treat as very shallow water
    }
    
    // Scale depth using heightmap scale from map file (64.0 for C2, 32.0 for C1)
    waterDepth = waterDepth / heightmapScale; // Convert to world units
    
    // Set reasonable depth range for transparency calculations
    float maxVisibleDepth = 3.0; // Reasonable depth range in world units
    waterDepth = clamp(waterDepth, 0.0, maxVisibleDepth);
    
    // Handle danger water (lava) differently
    if (isDangerWater == 1u) {
        // Subtle bubbling animation for lava effect
        float bubblingWaves = sin(time * 3.0 + texCoord0.x * 8.0) * 0.3 + 
                              cos(time * 2.5 + texCoord0.y * 6.0) * 0.2 +
                              sin(time * 4.0 + texCoord0.x * 4.0 + texCoord0.y * 4.0) * 0.15;
        
        // Subtle heat distortion offset for UV coordinates
        vec2 heatDistortion = vec2(
            sin(time * 2.0 + texCoord0.x * 15.0) * 0.002,
            cos(time * 2.5 + texCoord0.y * 12.0) * 0.002
        );
        
        // Sample texture with heat distortion
        vec4 sC = texture(basic_texture, texCoord0 + heatDistortion);
        
        // Calculate basic lighting for danger water
        vec3 unitSurfaceNormal = normalize(surfaceNormal);
        vec3 unitToLightVector = normalize(toLightVector);
        float diffuse = max(dot(unitSurfaceNormal, unitToLightVector), 0.0);
        float brightness = ambientStrength + diffuse * diffuseStrength;
        
        // Apply subtle lighting with heat glow
        brightness *= (1.0 + bubblingWaves * 0.1); // Subtle brightness variation from bubbling
        
        vec3 baseColor = vec3(sC.b, sC.g, sC.r) * brightness;
        
        // Subtle lava color enhancement - slightly warmer
        baseColor.r = min(baseColor.r * 1.2, 1.0);
        baseColor.g = min(baseColor.g * 1.05, 1.0);  
        baseColor.b *= 0.9; // Slightly reduce blue for warmer look
        
        // Skip complex transparency calculations - use high uniform opacity for lava
        float dangerAlpha = 0.9; // High opacity for lava
        
        // Basic fog calculation
        float dist = gl_FragCoord.z / gl_FragCoord.w;
        float min_distance = 256.0 * 6.0;
        float max_distance = view_distance;
        float fogFactor = 0.0;
        if (dist > min_distance) {
            fogFactor = clamp((dist - min_distance) / (max_distance - min_distance), 0.0, 1.0);
            fogFactor = min(fogFactor, 0.65);
        }
        
        vec3 finalColor = mix(baseColor, skyColor.rgb, fogFactor);
        outputColor = mix(vec4(finalColor, dangerAlpha), skyColor, 1.0 - EdgeFactor);
        return;
    }
    
    // Normal water processing continues below
    // Ambient and fade
    float min_distance = 256.0 * 6.0; // Start fog
    float max_distance = view_distance;
    float fogFactor = 0.0;
    float dist = gl_FragCoord.z / gl_FragCoord.w;

    vec4 sC = texture(basic_texture, texCoord0);

    if (dist > min_distance) {
        fogFactor = clamp((dist - min_distance) / (max_distance - min_distance), 0.0, 1.0);
        fogFactor = min(fogFactor, 0.65); // Limit the max fill so we keep things visible
    }

    // Lighting
    vec3 unitSurfaceNormal = normalize(surfaceNormal);
    vec3 unitToLightVector = normalize(toLightVector);

    float diffuse = max(dot(unitSurfaceNormal, unitToLightVector), 0.0);
    float brightness = ambientStrength + diffuse * diffuseStrength;

    // Sample the sky texture for cloud shadows
    vec4 cloudColor = texture(skyTexture, cloudTexCoord);
    float cloudLuminance = (0.299 * cloudColor.b + 0.587 * cloudColor.g + 0.114 * cloudColor.r) * 2.0;

    // Calculate wave influence on shadow intensity
    float waveInfluence = sin(10.0 + time * wave_speed) * wave_scale +
                          cos(10.0 + time * wave_speed) * wave_scale;

    // Modulate shadow intensity by vertex height (using surface normal y-component as proxy for height)
    float heightFactor = surfaceNormal.y; // Assuming surfaceNormal.y correlates with wave height
    heightFactor = clamp(heightFactor, 0.0, 1.0); // Ensure the factor is within [0, 1]

    // Adjust brightness based on cloud luminance, wave influence, and height
    float shadowIntensity = cloudLuminance + waveInfluence * heightFactor;

    brightness *= shadowIntensity;

    // Apply the lighting to the texture color
    vec3 baseColor = vec3(sC.b, sC.g, sC.r) * brightness;
    
    // Depth-based color tinting - shallow effects fade over 3 world units
    float shallowDepth = 3.0; // Shallow water effects fade over 3 world units (≈3 tiles)
    float depthColorFactor = clamp(waterDepth / shallowDepth, 0.0, 1.0);
    
    // Use actual water texture color for more natural look
    // Create shallow water color by brightening and slightly desaturating the base water texture
    vec3 shallowWaterColor = baseColor * 1.5; // Brighter version of the actual water texture
    shallowWaterColor = mix(shallowWaterColor, vec3(dot(shallowWaterColor, vec3(0.299, 0.587, 0.114))), -0.2); // Slight desaturation boost
    vec3 deepWaterColor = baseColor * 0.8; // Slightly darker for deep water
    
    // Blend between shallow and deep water colors based on depth
    vec3 depthTintedColor = mix(shallowWaterColor, deepWaterColor, depthColorFactor);
    
    // Mix in the sky color
    vec3 finalColor = mix(depthTintedColor, skyColor.rgb, fogFactor);

    // Depth-based and view-dependent transparency
    
    // Calculate viewing angle (0 = looking straight down, 1 = looking horizontally)
    vec3 normalizedView = normalize(viewDirection);
    vec3 normalizedSurface = normalize(surfaceNormal);
    float viewAngle = 1.0 - abs(dot(normalizedView, normalizedSurface));
    
    // Depth factor: fully opaque at 2 tiles depth
    // For 2 tiles = full opacity, use 2.0 as the scaling factor
    float depthFactor = clamp(waterDepth / 2.0, 0.0, 1.0); // Full opacity at 2 world units (≈2 tiles)
    
    // View-dependent transparency: more transparent when looking down, opaque at grazing angles
    float viewDependentTransparency = depthFactor * (1.0 - pow(viewAngle, 0.5));
    
    // Base alpha with distance-based modifications
    float baseAlpha = 0.9; // Reasonable base opacity
    if (dist > 200.0) {
      baseAlpha = clamp((dist - 200.0) / (512.0 - 200.0), 0.7, 0.9);
    }
    
    // Combine view-dependent transparency with base alpha - shallow to fully opaque
    // Use depthColorFactor (which fades over 1 world unit) for transparency too
    float shallowTransparencyFactor = clamp(waterDepth / shallowDepth, 0.0, 1.0); // 0 = fully transparent, 1 = opaque
    float minAlpha = mix(0.0, 1.0, shallowTransparencyFactor); // Fully transparent at 0 depth, opaque at 1+ world units
    float finalAlpha = mix(minAlpha, 1.0, viewDependentTransparency); // Max out at full opacity

    outputColor = mix(vec4(finalColor, finalAlpha), skyColor, 1.0 - EdgeFactor);
}
