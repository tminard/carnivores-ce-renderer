#version 330 core
layout(location = 0) in vec3 position; // Base position of the vertex, if applicable
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in float faceAlpha;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
out vec4 FragPosLightSpace;
out vec3 toLightVector;

uniform mat4 MVP;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;
uniform vec3 lightPosition;

void main()
{
    vec4 worldPos = model * vec4(position, 1.0);
    FragPos = vec3(worldPos);
    TexCoord = texCoord;
    
    // Transform normal to world space
    Normal = mat3(transpose(inverse(model))) * normal;
    
    // Transform position to light space for shadow mapping
    FragPosLightSpace = lightSpaceMatrix * worldPos;
    
    // Calculate light vector
    toLightVector = lightPosition - FragPos;
    
    gl_Position = MVP * vec4(position, 1.0);
}
