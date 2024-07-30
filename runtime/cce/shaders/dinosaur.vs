#version 330

layout(location = 0) in vec3 position; // Base position of the vertex, if applicable
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in float faceAlpha;
layout(location = 4) in mat4 instancedMatrix;

out vec2 texCoord0;
out float faceAlpha0;
out vec3 surfaceNormal;
out vec3 toLightVector;

uniform mat4 MVP;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Apply model and instanced transformations to the animated position
    vec4 worldPosition = model * instancedMatrix * vec4(position, 1.0);
    vec3 lightPosition = vec3(0.0, 0.0, 20000.0);

    // Transform normal to world space
    surfaceNormal = normalize(mat3(transpose(inverse(model))) * normal);

    // Calculate vector to light for lighting calculations
    toLightVector = lightPosition - worldPosition.xyz;

    // Apply view and projection transformations
    vec4 viewPosition = view * worldPosition;
    vec4 projectedPosition = projection * viewPosition;

    // Pass through texture coordinates and face alpha
    texCoord0 = texCoord;
    faceAlpha0 = faceAlpha;

    // Set the final position of the vertex
    gl_Position = projectedPosition;
}
