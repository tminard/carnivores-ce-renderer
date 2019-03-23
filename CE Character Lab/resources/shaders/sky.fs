#version 330 core
out vec4 outputColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    vec4 sC = texture(skybox, TexCoords);
    vec3 lightColor = vec3(sC.b, sC.g, sC.r);
    
    outputColor = vec4(lightColor, 1.0f);
}
