#version 330

in highp vec2 texCoord0;
in highp vec2 texCoord1;
in vec3 normal0;
smooth in float brightness0;
in vec2 quadCoord; // Coordinate to sample underwater state texture
in float wetness;  // Wetness factor from the vertex shader

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform float view_distance;
uniform vec4 distanceColor;

void main()
{
    // Visualize the normal as color
    vec3 normalColor = normalize(normal0) * 0.5 + 0.5; // Normalize and map to [0, 1] range

    outputColor = vec4(normalColor, 1.0);
}
