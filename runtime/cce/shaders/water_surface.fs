#version 330 core

in vec2 texCoord0;
smooth in float alpha0;
in float EdgeFactor;
in vec3 surfaceNormal;
in vec3 toLightVector;
in vec2 cloudTexCoord;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform sampler2D skyTexture;
uniform float time; // Time for wave animation
uniform vec4 skyColor;
uniform float view_distance;

uniform float ambientStrength = 0.45;
uniform float diffuseStrength = 0.55;

const float wave_speed = 0.1;
const float wave_scale = 0.02;

void main()
{
    // ambient and fade
    float min_distance = 256.0 * 6.0; // Start fog
    float max_distance = view_distance;
    float fogFactor = 0.0;
    float distance = gl_FragCoord.z / gl_FragCoord.w;

    vec4 sC = texture(basic_texture, texCoord0);

    if (distance > min_distance) {
        fogFactor = clamp((distance - min_distance) / (max_distance - min_distance), 0.0, 1.0);
        fogFactor = min(fogFactor, 0.65); // Limit the max fill so we keep things visible
    } else {
        // Apply wave distortion to texture coordinates
        vec2 wave = texCoord0 + vec2(sin(texCoord0.x * 10.0 + time * wave_speed) * wave_scale,
                                     cos(texCoord0.y * 10.0 + time * wave_speed) * wave_scale);
        
        sC = texture(basic_texture, wave);
    }

    // Lighting
    vec3 unitSurfaceNormal = normalize(surfaceNormal);
    vec3 unitToLightVector = normalize(toLightVector);

    float diffuse = max(dot(unitSurfaceNormal, unitToLightVector), 0.0);
    float brightness = ambientStrength + diffuse * diffuseStrength;

    // Apply distortion to cloud texture coordinates
    vec2 distortedCloudTexCoord = cloudTexCoord + vec2(sin(cloudTexCoord.x * 10.0 + time * wave_speed) * wave_scale,
                                                       cos(cloudTexCoord.y * 10.0 + time * wave_speed) * wave_scale);
    
    // Sample the sky texture for cloud shadows using distorted cloud texture coordinates
    vec4 cloudColor = texture(skyTexture, distortedCloudTexCoord);
    float cloudLuminance = (0.299 * cloudColor.b + 0.587 * cloudColor.g + 0.114 * cloudColor.r) * 2.0;

    // Adjust brightness based on cloud luminance
    brightness *= cloudLuminance;

    // Apply the lighting to the texture color
    vec3 finalColor = vec3(sC.b, sC.g, sC.r) * brightness;

    // Mix in the sky color
    finalColor = mix(finalColor, skyColor.rgb, fogFactor);
   
    // Ensure the alpha does not go below factor
    float finalAlpha = max(alpha0, 0.15);

    outputColor = mix(vec4(finalColor, finalAlpha), skyColor, 1.0 - EdgeFactor);
}
