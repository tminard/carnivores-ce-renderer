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
out vec3 worldPos;
flat out uint fogType;

uniform mat4 MVP;
uniform mat4 model;
uniform float terrainWidth;
uniform float terrainHeight;
uniform float tileWidth;
uniform float time;
uniform vec3 cameraPos;

const float waveAmplitude = 8.0;  // Gentler waves for fog
const float waveFrequency = 6.0;  // Lower frequency for smoother motion
const float waveSpeed = 1.5;      // Slower movement for fog

void main()
{
    // Apply subtle animated displacement for fog movement
    vec3 animatedPosition = position;
    vec3 lightPosition = vec3((tileWidth * terrainWidth * 0.5), (tileWidth * terrainHeight * 0.5), 20000.0);

    // Add layered noise for volumetric fog movement
    float noiseX = sin(position.x * 0.02 + time * 0.5) * 0.3 + 
                   cos(position.x * 0.015 + time * 0.3) * 0.2;
    float noiseZ = cos(position.z * 0.02 + time * 0.4) * 0.3 + 
                   sin(position.z * 0.018 + time * 0.6) * 0.2;
    float noiseY = sin(position.x * 0.01 + position.z * 0.01 + time * 0.8) * 0.15;

    // Apply noise displacement - more subtle than water
    animatedPosition.x += noiseX * waveAmplitude * 0.5;
    animatedPosition.z += noiseZ * waveAmplitude * 0.5;
    animatedPosition.y += noiseY * waveAmplitude * 0.3;

    vec4 worldPosition = model * vec4(animatedPosition, 1.0);
    worldPos = worldPosition.xyz;

    surfaceNormal = mat3(transpose(inverse(model))) * normal;
    toLightVector = lightPosition - worldPosition.xyz;

    gl_Position = MVP * worldPosition;
    
    // Calculate distance to the edge of the map for fading
    float edgeDistanceX = min(position.x, (terrainWidth * tileWidth) - position.x);
    float edgeDistanceY = min(position.z, (terrainHeight * tileWidth) - position.z);
    float minEdgeDistance = min(edgeDistanceX, edgeDistanceY);
    float maxEdgeDistance = tileWidth * 40.0; // Softer edge fade for fog
    EdgeFactor = clamp(minEdgeDistance / maxEdgeDistance, 0.0, 1.0);
    
    texCoord0 = texCoord;
    alpha0 = alpha;

    // Calculate cloud texture coordinates with different movement for fog
    cloudTexCoord = vec2(
        1.0 - (position.z / (tileWidth * 128.0)) - (time * 0.002), 
        1.0 - (position.x / (tileWidth * 128.0)) - (time * 0.003)
    );
    
    // Calculate view direction for fog effects
    viewDirection = normalize(cameraPos - worldPosition.xyz);
    
    // Pass fog type information (1 = danger/lava fog, 0 = normal fog)
    fogType = flags;
}