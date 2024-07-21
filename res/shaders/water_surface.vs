#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal; // Assuming normal data is passed as location 2
layout(location = 3) in float alpha;

out vec2 texCoord0;
out vec3 normal0;
out vec3 fragPos;
smooth out float alpha0;

uniform mat4 MVP;
uniform mat4 model;
uniform float RealTime;

const float waveAmplitude = 0.05;
const float waveFrequency = 2.0;
const float waveSpeed = 1.0;

void main()
{
    // Apply wave animation to the vertex position
    vec3 animatedPosition = position;
    animatedPosition.z += sin(position.x * waveFrequency + RealTime * waveSpeed) * waveAmplitude;
    animatedPosition.z += cos(position.y * waveFrequency + RealTime * waveSpeed) * waveAmplitude;

    vec4 worldPosition = model * vec4(animatedPosition, 1.0);
    gl_Position = MVP * worldPosition;
    
    texCoord0 = texCoord;
    normal0 = mat3(model) * normal; // Transform normal to world space
    fragPos = worldPosition.xyz; // Pass the fragment position to the fragment shader
    alpha0 = alpha;
}
