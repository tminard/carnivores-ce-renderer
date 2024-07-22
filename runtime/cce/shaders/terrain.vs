#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 texCoords;
layout(location = 3) in float brightness; // 0 = max bright; 55 = max dark

out highp vec2 texCoord0;
out highp vec2 texCoord1;
out vec3 normal0;
smooth out float brightness0;

uniform mat4 MVP;

const vec4 plane = vec4(0, 0, -1, 10);

void main()
{
    vec4 worldPosition = MVP * vec4(position, 1);

    texCoord0 = vec2(texCoords[0], texCoords[1]);;
    texCoord1 = vec2(texCoords[2], texCoords[3]);
    brightness0 = brightness;
    normal0 = normal;

    gl_Position = worldPosition;
    gl_ClipDistance[0] = dot(worldPosition, plane);
}
