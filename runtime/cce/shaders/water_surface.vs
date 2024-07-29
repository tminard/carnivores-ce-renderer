#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in float alpha;

out vec2 texCoord0;
smooth out float alpha0;
out float EdgeFactor;
out vec3 surfaceNormal;
out vec3 toLightVector;
out vec2 cloudTexCoord;

uniform mat4 MVP;
uniform mat4 model;
uniform float terrainWidth;
uniform float terrainHeight;
uniform float tileWidth;
uniform float time;
uniform vec3 cameraPos;

const float waveAmplitude = 6.0; // Increased amplitude for more pronounced waves
const float waveFrequency = 5.0; // Increased frequency for more ripples
const float waveSpeed = 2.0;     // Increased speed for faster ripples
const float maxDistance = 128.0 * 30.0; // Maximum distance for wave animation

void main()
{
    // Apply wave animation to the vertex position
    vec3 animatedPosition = position;
    vec3 lightPosition = vec3((tileWidth * terrainWidth * 0.5), (tileWidth * terrainHeight * 0.5), 20000.0);

    if (alpha > 0.95) {
        // Only animate the main body of the surface and not the edges or shallow pools
        animatedPosition.y += sin(position.x * waveFrequency + time * waveSpeed) * waveAmplitude;
        animatedPosition.y += cos(position.z * waveFrequency + time * waveSpeed) * waveAmplitude;
    }

    vec4 worldPosition = model * vec4(animatedPosition, 1.0);

    surfaceNormal = mat3(transpose(inverse(model))) * normal; // Transform normal to world space
    toLightVector = lightPosition - worldPosition.xyz;

    gl_Position = MVP * worldPosition;
    
    // Calculate distance to the edge of the map
    float edgeDistanceX = min(position.x, (terrainWidth * tileWidth) - position.x);
    float edgeDistanceY = min(position.z, (terrainHeight * tileWidth) - position.z);
    float minEdgeDistance = min(edgeDistanceX, edgeDistanceY);
    float maxEdgeDistance = tileWidth * 60.0;
    EdgeFactor = clamp(minEdgeDistance / maxEdgeDistance, 0.0, 1.0);
    
    texCoord0 = texCoord;
    alpha0 = alpha;

    // Calculate cloud texture coordinates
    cloudTexCoord = vec2(1.0 - (position.z / (tileWidth * 128.0)) - (time * 0.008), 1.0 - (position.x / (tileWidth * 128.0)) - (time * 0.008));
}
