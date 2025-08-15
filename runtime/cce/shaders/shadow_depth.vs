#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    vec4 worldPos = model * vec4(position, 1.0);
    gl_Position = lightSpaceMatrix * worldPos;
    
    // Remove the clamp for now to see if it's causing issues
    // gl_Position.xyz = clamp(gl_Position.xyz, vec3(-10.0), vec3(10.0));
}