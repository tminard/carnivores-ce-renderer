#version 330

in highp vec2 texCoord0;
in highp vec2 texCoord1;
in vec3 surfaceNormal;
in vec3 toLightVector;
in vec2 quadCoord;
in float wetness;
in highp vec2 out_textCoord_clouds;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform sampler2D skyTexture;
uniform float view_distance;
uniform vec4 distanceColor;

uniform float ambientStrength = 0.05;
uniform float diffuseStrength = 0.95;

void main()
{
    vec4 sC;

    // Determine which texture coordinates to use for the current face
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

    vec4 cloudColor = texture(skyTexture, out_textCoord_clouds);
    float cloudLuminance = (0.299 * cloudColor.b + 0.587 * cloudColor.g + 0.114 * cloudColor.r) * 2.0;

    float brightness = (ambientStrength + pow(diffuse, 1.0) * diffuseStrength) * pow(cloudLuminance, 1.75);

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
