#version 330 core

in vec2 texCoord0;
smooth in float alpha0;
in float EdgeFactor;
in vec3 surfaceNormal;
in vec3 toLightVector;
in vec2 cloudTexCoord;
in vec3 viewDirection;
in vec3 worldPos;
flat in uint fogType;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform sampler2D skyTexture;
uniform float time;
uniform vec4 skyColor;
uniform float view_distance;
uniform vec3 cameraPos;
uniform float fogTransparency;
uniform vec3 fogColor;

uniform float ambientStrength = 0.3;
uniform float diffuseStrength = 0.4;

void main()
{
    // Calculate distance from camera for depth-based effects
    float distanceFromCamera = length(worldPos - cameraPos);
    
    // Sample noise texture for volumetric density variation
    vec2 noiseCoord1 = texCoord0 * 4.0 + vec2(time * 0.02, time * 0.015);
    vec2 noiseCoord2 = texCoord0 * 2.0 + vec2(time * -0.01, time * 0.025);
    vec4 noise1 = texture(basic_texture, noiseCoord1);
    vec4 noise2 = texture(basic_texture, noiseCoord2);
    
    // Combine noise for density variation
    float density = (noise1.r * 0.6 + noise2.g * 0.4) * 0.8 + 0.2;
    
    // Calculate base lighting
    vec3 unitSurfaceNormal = normalize(surfaceNormal);
    vec3 unitToLightVector = normalize(toLightVector);
    float diffuse = max(dot(unitSurfaceNormal, unitToLightVector), 0.0);
    float brightness = ambientStrength + diffuse * diffuseStrength;
    
    // Sample cloud texture for atmospheric variation
    vec4 cloudColor = texture(skyTexture, cloudTexCoord);
    float cloudLuminance = (0.299 * cloudColor.b + 0.587 * cloudColor.g + 0.114 * cloudColor.r);
    brightness *= (0.7 + cloudLuminance * 0.3);
    
    vec3 baseColor;
    float fogAlpha;
    
    // More subtle, swirl-like atmospheric effects
    float swirl = sin(worldPos.x * 0.02 + time * 0.3) * 0.08 + 
                  cos(worldPos.z * 0.015 + time * 0.25) * 0.06 +
                  sin((worldPos.x + worldPos.z) * 0.01 + time * 0.2) * 0.05;
    
    // Use the actual fog color from map data as primary color
    baseColor = fogColor;
    // Add very subtle atmospheric variation using swirl
    baseColor = mix(baseColor, baseColor * 1.1, swirl * 0.1); // Subtle brightness variation
    fogAlpha = fogTransparency * density * 1.2; // Moderate density increase
    
    if (fogType == 1u) {
        // Danger fog - ADD heat effects on top of atmospheric base
        
        // Heat shimmer effect
        float heatIntensity = sin(time * 4.0 + worldPos.x * 0.1) * 0.3 + 
                              cos(time * 3.5 + worldPos.z * 0.12) * 0.2 +
                              sin(time * 5.0 + worldPos.y * 0.08) * 0.15;
        
        // Animated embers/sparks effect
        vec2 emberCoord = texCoord0 * 8.0 + vec2(time * 0.1, time * -0.15);
        float emberNoise = texture(basic_texture, emberCoord).r;
        float emberEffect = step(0.85, emberNoise) * heatIntensity * 0.3; // Reduced intensity
        
        // Enhance the fog color with heat effects while preserving base color
        baseColor = mix(baseColor, baseColor * (1.0 + heatIntensity * 0.3), 0.4); // Subtler heat enhancement
        baseColor.r = min(baseColor.r * (1.05 + emberEffect * 0.5), 1.0); // Subtle warming
        baseColor.g = min(baseColor.g * (1.02 + emberEffect * 0.2), 1.0);
        baseColor.b = min(baseColor.b * (0.95 + emberEffect * 0.1), 1.0); // Less blue reduction
        
        // Slightly higher opacity for danger fog
        fogAlpha *= 1.1;
    }
    
    // Apply lighting to base color
    baseColor *= brightness;
    
    // Distance-based opacity fade
    float distanceFade = 1.0 - clamp((distanceFromCamera - 100.0) / (view_distance - 100.0), 0.0, 1.0);
    fogAlpha *= distanceFade;
    
    // View angle dependent transparency - fog is more transparent when looking through it
    vec3 normalizedView = normalize(viewDirection);
    vec3 normalizedSurface = normalize(surfaceNormal);
    float viewAngle = abs(dot(normalizedView, normalizedSurface));
    fogAlpha *= (0.3 + viewAngle * 0.7); // More transparent at grazing angles
    
    // Apply edge fading
    fogAlpha *= EdgeFactor;
    
    // Use pure fog color with no sky blending to show true map data colors
    vec3 finalColor = baseColor;
    
    outputColor = vec4(finalColor, clamp(fogAlpha, 0.0, 0.9));
}