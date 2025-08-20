#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 texCoords;

out highp vec2 texCoord0;
out highp vec2 texCoord1;
out vec3 surfaceNormal;
out vec3 toLightVector;
out vec2 quadCoord;
out float wetness;
out highp vec2 out_textCoord_clouds;
out vec3 FragPos;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float terrainWidth;
uniform float terrainHeight;
uniform float tileWidth;
uniform sampler2D underwaterStateTexture;
uniform highp float time;
uniform mat4 lightSpaceMatrix;
uniform bool enableShadows = false;
uniform vec3 lightPosition;

void main()
{
    vec4 worldPosition = model * vec4(position, 1.0);

    // Calculate quad coordinates for sampling water height texture
    quadCoord = vec2(position.x / tileWidth, position.z / tileWidth) / vec2(terrainWidth, terrainHeight);

    // Sample water height and calculate wetness factor
    float waterHeight = texture(underwaterStateTexture, quadCoord).r;
    wetness = clamp((waterHeight - position.y) / tileWidth, 0.0, 1.0);

    surfaceNormal = mat3(transpose(inverse(model))) * normal; // Transform normal to world space
    toLightVector = lightPosition - worldPosition.xyz; // Use world object light position

    vec4 viewPosition = view * worldPosition;
    vec4 projectedPosition = projection * viewPosition;

    texCoord0 = vec2(texCoords[0], texCoords[1]);
    texCoord1 = vec2(texCoords[2], texCoords[3]);

    // Calculate cloud texture coordinates
    out_textCoord_clouds = vec2(1.0 - (position.z / (tileWidth * 128.0)) - (time * 0.008), 1.0 - (position.x / (tileWidth * 128.0)) - (time * 0.008));

    FragPos = worldPosition.xyz;
    
    if (enableShadows) {
        FragPosLightSpace = lightSpaceMatrix * worldPosition;
    }

    gl_Position = projectedPosition;
}
