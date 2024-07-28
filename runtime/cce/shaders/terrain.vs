#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 texCoords;

// Pre-baked original lightmap data in case we want it
// float percent = (brightness0 / 255.0);
layout(location = 3) in float brightness; // 0 = max bright; 55 = max dark

out highp vec2 texCoord0;
out highp vec2 texCoord1;
out vec3 surfaceNormal;
out vec3 toLightVector;
out vec2 quadCoord;
out float wetness;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float terrainWidth;
uniform float terrainHeight;
uniform float tileWidth;
uniform sampler2D underwaterStateTexture;
uniform float time;

void main()
{
    vec4 worldPosition = model * vec4(position, 1.0);
    vec3 lightPosition = vec3((tileWidth * terrainWidth * 0.5), 20000.0, (tileWidth * terrainHeight * 0.5));

    // Calculate quad coordinates for sampling water height texture
    quadCoord = vec2(position.x / tileWidth, position.z / tileWidth) / vec2(terrainWidth, terrainHeight);

    // Sample water height and calculate wetness factor
    float waterHeight = texture(underwaterStateTexture, quadCoord).r;
    wetness = clamp((waterHeight - position.y) / tileWidth, 0.0, 1.0);

    surfaceNormal = mat3(transpose(inverse(model))) * normal; // Transform normal to world space
    toLightVector = lightPosition - worldPosition.xyz;

    vec4 viewPosition = view * worldPosition;
    vec4 projectedPosition = projection * viewPosition;

    texCoord0 = vec2(texCoords[0], texCoords[1]);
    texCoord1 = vec2(texCoords[2], texCoords[3]);

    gl_Position = projectedPosition;
}
