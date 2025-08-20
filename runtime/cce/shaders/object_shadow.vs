#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 uv;
layout(location = 3) in mat4 instancedMatrix;

out vec3 FragPos;
out vec2 TexCoord;
out vec4 FragPosLightSpace;
out vec3 toLightVector;

uniform mat4 projection_view;
uniform mat4 lightSpaceMatrix;
uniform vec3 lightPosition;

void main()
{
    vec4 worldPos = instancedMatrix * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);
    TexCoord = uv;
    
    // Transform position to light space for shadow mapping
    FragPosLightSpace = lightSpaceMatrix * worldPos;
    
    // Calculate light vector
    toLightVector = lightPosition - FragPos;
    
    gl_Position = projection_view * worldPos;
}