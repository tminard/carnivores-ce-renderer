#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 TexCoords;

out vec2 TexCoords0;
out vec3 WorldPos;

uniform mat4 projection;
uniform mat4 view;
uniform float time;

void main()
{
    // Very slow cloud movement for distant sky effect
    TexCoords0 = TexCoords + vec2(time * 0.001, time * 0.0005);
    
    // Pass world position for circular fade calculation
    WorldPos = aPos;

    gl_Position = projection * view * vec4(aPos, 1.0);
}
