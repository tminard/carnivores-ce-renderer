#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 TexCoords;

out vec2 TexCoords0;

uniform mat4 projection;
uniform mat4 view;
uniform float time;

void main()
{
    TexCoords0 = TexCoords + vec2(time * 0.005, time * 0.005) / 3.0;

    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = projection * view * vec4(aPos, 1.0);
}
