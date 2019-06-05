#version 330

layout(location = 0) in vec3 position;
layout(location = 2) in vec4 texCoords;
layout(location = 3) in float brightness; // 0 = max bright; 55 = max dark

out highp vec2 texCoord0;
out highp vec2 texCoord1;
smooth out float brightness0;

uniform mat4 MVP;

void main()
{
    vec4 v = vec4(position, 1);
    gl_Position = MVP * v;
    texCoord0 = vec2(texCoords[0], texCoords[1]);;
    texCoord1 = vec2(texCoords[2], texCoords[3]);
    brightness0 = brightness;
}
