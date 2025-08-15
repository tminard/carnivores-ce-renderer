#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 uv;
layout (location = 3) in mat4 instancedMatrix;

out vec2 texCoord0;
out vec3 FragPos;
out vec4 FragPosLightSpace;

uniform mat4 MVP;
uniform mat4 projection_view;
uniform mat4 lightSpaceMatrix;
uniform bool enableShadows = false;

void main()
{
    vec4 v = vec4(aPos, 1);
    vec4 worldPos = instancedMatrix * v;
    vec4 pos = projection_view * worldPos;

    texCoord0 = uv;
    FragPos = worldPos.xyz;
    
    if (enableShadows) {
        FragPosLightSpace = lightSpaceMatrix * worldPos;
    }
    
    gl_Position = pos;
}
