#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in float faceAlpha;
layout(location = 4) in mat4 instancedMatrix;

out vec2 texCoord0;
out float faceAlpha0;
out vec3 surfaceNormal;
out vec3 toLightVector;

uniform highp mat4 MVP;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float terrainWidth;
uniform float terrainHeight;
uniform float tileWidth;
uniform vec3 cameraPos;
uniform float time;

void main()
{
    vec4 worldPosition = model * instancedMatrix * vec4(position, 1.0);
    vec3 lightPosition = vec3((tileWidth * terrainWidth * 0.5), 20000.0, (tileWidth * terrainHeight * 0.5));

    surfaceNormal = mat3(transpose(inverse(model))) * normal; // Transform normal to world space
    toLightVector = lightPosition - worldPosition.xyz;

    vec4 viewPosition = view * worldPosition;
    vec4 projectedPosition = projection * viewPosition;

    texCoord0 = texCoord;
    faceAlpha0 = faceAlpha;

    gl_Position = projectedPosition;
}
