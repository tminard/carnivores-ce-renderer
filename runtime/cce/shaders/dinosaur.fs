#version 330

in vec2 texCoord0;
in float faceAlpha0;
in vec3 surfaceNormal;
in vec3 toLightVector;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform bool enable_transparency;
uniform float view_distance;
uniform vec4 distanceColor;

uniform float ambientStrength = 0.8;
uniform float diffuseStrength = 0.2;
uniform float time;

void main()
{
    vec4 sC = texture(basic_texture, texCoord0);

    // Lighting
    vec3 unitSurfaceNormal = normalize(surfaceNormal);
    vec3 unitToLightVector = normalize(toLightVector);

    float diffuse = max(dot(unitSurfaceNormal, unitToLightVector), 0.0);
    float brightness = ambientStrength + diffuse * diffuseStrength;

    // Apply the lighting to the texture color
    vec3 finalColor = vec3(sC.b, sC.g, sC.r) * brightness;

    // Fog effect
    float min_distance = view_distance * 0.50; // Start fog at half distance
    float max_distance = view_distance;
    float fogFactor = 0.0;

    float distance = gl_FragCoord.z / gl_FragCoord.w;

    if (distance > min_distance) {
        fogFactor = clamp((distance - min_distance) / (max_distance - min_distance), 0.0, 1.0);
        fogFactor = min(fogFactor, 0.45); // Limit the max fill so we keep things visible
    }

    finalColor = mix(finalColor, distanceColor.rgb, fogFactor);
    
    // Transparency discard
    float trans = 0.095;
    if (faceAlpha0 == 0.0 && enable_transparency && sC.r <= trans && sC.g <= trans && sC.b <= trans) {
        discard;
    }

    outputColor = vec4(finalColor, 1.0);
}
