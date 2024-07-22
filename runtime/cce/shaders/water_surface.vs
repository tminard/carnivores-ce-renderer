#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal; // Assuming normal data is passed as location 2
layout(location = 3) in float alpha;

out vec2 texCoord0;
smooth out float alpha0;

uniform mat4 MVP;
uniform mat4 model;
uniform float RealTime;

const float waveAmplitude = 6.0; // Increased amplitude for more pronounced waves
const float waveFrequency = 5.0; // Increased frequency for more ripples
const float waveSpeed = 2.0;     // Increased speed for faster ripples

void main()
{
    // Apply wave animation to the vertex position
    vec3 animatedPosition = position;
    animatedPosition.y += sin(position.x * waveFrequency + RealTime * waveSpeed) * waveAmplitude;
    animatedPosition.y += cos(position.z * waveFrequency + RealTime * waveSpeed) * waveAmplitude;

    vec4 worldPosition = model * vec4(animatedPosition, 1.0);
    gl_Position = MVP * worldPosition;
    
    texCoord0 = texCoord;
    alpha0 = alpha;
}
