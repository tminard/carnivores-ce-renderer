#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in float alpha;
layout(location = 4) in uint flags;

out vec2 texCoord0;
smooth out float alpha0;
out float EdgeFactor;
out vec3 surfaceNormal;
out vec3 toLightVector;
out vec2 cloudTexCoord;
out vec3 viewDirection;
out vec2 heightmapCoord;
flat out uint isDangerWater;

uniform mat4 MVP;
uniform mat4 model;
uniform float terrainWidth;
uniform float terrainHeight;
uniform float tileWidth;
uniform float time;
uniform vec3 cameraPos;
uniform float waterLevel;
uniform sampler2D heightmapTexture;

const float waveAmplitude = 0.75; // Scaled down 16x for new world scale (was 12.0)
const float waveFrequency = 0.625; // Scaled down 16x for new world scale (was 10.0)
const float waveSpeed = 2.0;     // Keep same speed for natural animation
const float maxDistance = 16.0 * 30.0; // Scaled down 16x for new world scale (was 128.0 * 30.0)

void main()
{
    // Apply depth-dependent wave animation to the vertex position
    vec3 animatedPosition = position;
    vec3 lightPosition = vec3((tileWidth * terrainWidth * 0.5), (tileWidth * terrainHeight * 0.5), 20000.0);

    if (alpha > 0.90) {
        // Calculate heightmap coordinates for depth sampling
        vec2 heightCoord = vec2(position.x / (terrainWidth * tileWidth), position.z / (terrainHeight * tileWidth));
        float terrainHeight = texture(heightmapTexture, heightCoord).r;
        float waterDepth = waterLevel - terrainHeight;
        
        // Depth-based wave attenuation: deeper water has larger waves, shallow water has smaller waves
        float depthFactor = clamp(waterDepth / 6.25, 0.1, 1.0); // Scaled down 16x (was 100.0)
        float attenuatedAmplitude = waveAmplitude * depthFactor;
        
        // Only animate the main body of the surface and not the edges or shallow pools
        animatedPosition.y += sin(position.x * waveFrequency + time * waveSpeed) * attenuatedAmplitude;
        animatedPosition.y += cos(position.z * waveFrequency + time * waveSpeed) * attenuatedAmplitude;
    }

    vec4 worldPosition = model * vec4(animatedPosition, 1.0);

    surfaceNormal = mat3(transpose(inverse(model))) * normal; // Transform normal to world space
    toLightVector = lightPosition - worldPosition.xyz;

    gl_Position = MVP * worldPosition;
    
    // Calculate distance to the edge of the map
    float edgeDistanceX = min(position.x, (terrainWidth * tileWidth) - position.x);
    float edgeDistanceY = min(position.z, (terrainHeight * tileWidth) - position.z);
    float minEdgeDistance = min(edgeDistanceX, edgeDistanceY);
    float maxEdgeDistance = tileWidth * 60.0; // This auto-scales with tileWidth
    EdgeFactor = clamp(minEdgeDistance / maxEdgeDistance, 0.0, 1.0);
    
    texCoord0 = texCoord;
    alpha0 = alpha;

    // Calculate cloud texture coordinates (auto-scales with tileWidth)
    cloudTexCoord = vec2(1.0 - (position.z / (tileWidth * 128.0)) - (time * 0.004), 1.0 - (position.x / (tileWidth * 128.0)) - (time * 0.004));
    
    // Calculate view direction for depth-dependent transparency
    viewDirection = normalize(cameraPos - worldPosition.xyz);
    
    // Calculate heightmap texture coordinates (normalized to [0,1])
    heightmapCoord = vec2(position.x / (terrainWidth * tileWidth), position.z / (terrainHeight * tileWidth));
    
    // Pass danger water flag (1 = danger water, 0 = normal water)
    isDangerWater = flags;
}
