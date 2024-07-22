#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal; // Assuming normal data is passed as location 2
layout(location = 3) in float alpha;

out vec2 texCoord0;
smooth out float alpha0;
out float EdgeFactor;

uniform mat4 MVP;
uniform mat4 model;
uniform float RealTime;
uniform vec3 mapDimensions; // x = width, y = height, z = tile_size
uniform vec3 cameraPos;

const float waveAmplitude = 6.0; // Increased amplitude for more pronounced waves
const float waveFrequency = 5.0; // Increased frequency for more ripples
const float waveSpeed = 2.0;     // Increased speed for faster ripples
const float maxDistance = 128.0 * 30.0; // Maximum distance for wave animation

void main()
{
    // Apply wave animation to the vertex position
    vec3 animatedPosition = position;

    animatedPosition.y += sin(position.x * waveFrequency + RealTime * waveSpeed) * waveAmplitude;
    animatedPosition.y += cos(position.z * waveFrequency + RealTime * waveSpeed) * waveAmplitude;

    vec4 worldPosition = model * vec4(animatedPosition, 1.0);
    gl_Position = MVP * worldPosition;
    
    // Calculate distance to the edge of the map
    float edgeDistanceX = min(position.x, mapDimensions.x - position.x);
    float edgeDistanceY = min(position.z, mapDimensions.y - position.z);
    float minEdgeDistance = min(edgeDistanceX, edgeDistanceY);
    float maxEdgeDistance = mapDimensions.z * 60.0;
    EdgeFactor = clamp(minEdgeDistance / maxEdgeDistance, 0.0, 1.0);
    
    texCoord0 = texCoord;
    alpha0 = alpha;
}
