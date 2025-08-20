#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aInstancePos;
layout (location = 3) in float aInstanceSize;
layout (location = 4) in vec3 aInstanceColor;

uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;
out vec3 ParticleColor;
out float Alpha;

void main()
{
    // Billboard the particle to face the camera
    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);
    
    vec3 worldPos = aInstancePos + (aPos.x * cameraRight + aPos.y * cameraUp) * aInstanceSize;
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
    TexCoord = aTexCoord;
    ParticleColor = aInstanceColor;
    Alpha = 1.0; // Will be modified by particle life in fragment shader
}