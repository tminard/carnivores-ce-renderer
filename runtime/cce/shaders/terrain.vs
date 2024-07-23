#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 texCoords;
layout(location = 3) in float brightness; // 0 = max bright; 55 = max dark

out highp vec2 texCoord0;
out highp vec2 texCoord1;
out vec3 normal0;
smooth out float brightness0;   
out vec2 quadCoord; // Coordinate to sample underwater state texture
out float wetness; // Wetness factor

uniform mat4 MVP;
uniform float terrainWidth;
uniform float terrainHeight;
uniform float tileWidth;
uniform sampler2D underwaterStateTexture;
uniform float time; // Add a time variable for animation

const vec4 plane = vec4(0, 0, -1, 10);

const float waveAmplitude = 6.0; // Increased amplitude for more pronounced waves
const float waveFrequency = 5.0; // Increased frequency for more ripples
const float waveSpeed = 2.0;     // Increased speed for faster ripples

void main()
{
    vec4 worldPosition = vec4(position, 1.0);

    // Calculate quad coordinates for sampling water height texture
    quadCoord = vec2(position.x / tileWidth, position.z / tileWidth) / vec2(terrainWidth, terrainHeight);

    // Sample water height and calculate wetness factor
    float waterHeight = texture(underwaterStateTexture, quadCoord).r;
    wetness = clamp((waterHeight - position.y) / tileWidth, 0.0, 1.0);

    // Apply ripple effect if underwater
    //if (wetness > 0.0) {
    //    vec3 animatedPosition = position;
//
//        animatedPosition.y += sin(position.x * waveFrequency + time * waveSpeed) * waveAmplitude;
 //       animatedPosition.y += cos(position.z * waveFrequency + time * waveSpeed) * waveAmplitude;
//
 //       worldPosition = vec4(animatedPosition, 1.0);
 //   }

    worldPosition = MVP * worldPosition;

    texCoord0 = vec2(texCoords[0], texCoords[1]);
    texCoord1 = vec2(texCoords[2], texCoords[3]);
    brightness0 = brightness;
    normal0 = normal;

    // Apply a small depth bias to avoid z-fighting
    worldPosition.y += 0.01 * worldPosition.w;

    gl_Position = worldPosition;
}
