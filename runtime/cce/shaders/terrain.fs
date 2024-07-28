#version 330

in highp vec2 texCoord0;
in highp vec2 texCoord1;
in vec3 surfaceNormal;
in vec3 toLightVector;
in vec2 quadCoord; // Coordinate to sample underwater state texture
in float wetness;  // Wetness factor from the vertex shader

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform float view_distance;
uniform vec4 distanceColor;

uniform float ambientStrength = 0.45;
uniform float diffuseStrength = 0.55;

void main()
{
    vec4 sC;

    // to use the first or second texture ID for the triangle face. Just a naive round-robin.
    // C1 only. FIXME BUG: the order (which face is 1 vs 2) is dependent on something (perhaps rotation or "order" flag).
    if (mod(gl_PrimitiveID, 2) == 0) {
        sC = texture(basic_texture, texCoord1);
    } else {
        sC = texture(basic_texture, texCoord0);
    }

    // Ambient and fade
    float min_distance = view_distance * 0.50; // Start fog at half distance
    float max_distance = view_distance;
    float fogFactor = 0.0;
    float distance = gl_FragCoord.z / gl_FragCoord.w;

    if (distance > min_distance) {
        fogFactor = clamp((distance - min_distance) / (max_distance - min_distance), 0.0, 1.0);
        fogFactor = min(fogFactor, 0.45); // Limit the max fill so we keep things visible
    }

    // Lighting
    vec3 unitSurfaceNormal = normalize(surfaceNormal);
    vec3 unitToLightVector = normalize(toLightVector);

    float diffuse = max(dot(unitSurfaceNormal, unitToLightVector), 0.0);
    float brightness = ambientStrength + diffuse * diffuseStrength;

    // Apply the lighting to the texture color
    vec3 finalColor = vec3(sC.b, sC.g, sC.r) * brightness;

    finalColor = mix(finalColor, distanceColor.rgb, fogFactor);

    // Apply a wetness effect
    // if (wetness > 0.0) {
    //     finalColor = mix(finalColor, distanceColor.rgb, wetness * 0.10);
    //     // alpha *= mix(1.0, 0.5, wetness); // Adjust alpha for underwater visibility
    // }

    outputColor = vec4(finalColor, 1.0);
}
