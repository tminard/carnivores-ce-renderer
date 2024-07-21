#version 330

in vec2 texCoord0;
in vec3 normal0;
in vec3 fragPos;
smooth in float alpha0;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform sampler2D reflection_texture; // Texture for reflection
uniform sampler2D refraction_texture; // Texture for refraction
uniform sampler2D normal_map; // Normal map for water surface
uniform vec3 light_dir; // Direction of the light source
uniform vec3 view_dir; // Direction of the camera view
uniform float time; // Time for wave animation

const float wave_speed = 0.1;
const float wave_scale = 0.02;
const float fresnel_power = 2.0;

void main()
{
    // Apply wave distortion to texture coordinates
    vec2 wave = texCoord0 + vec2(sin(texCoord0.x * 10.0 + time * wave_speed) * wave_scale,
                                 cos(texCoord0.y * 10.0 + time * wave_speed) * wave_scale);
    vec4 sC = texture(basic_texture, wave);

    // Calculate normal from normal map
    vec3 normal = texture(normal_map, wave).rgb * 2.0 - 1.0;

    // Calculate reflection and refraction
    vec3 reflect_dir = reflect(-view_dir, normal);
    vec3 refract_dir = refract(-view_dir, normal, 1.0 / 1.33); // Assuming water refraction index is 1.33

    vec4 reflection = texture(reflection_texture, reflect_dir.xy * 0.5 + 0.5);
    vec4 refraction = texture(refraction_texture, refract_dir.xy * 0.5 + 0.5);

    // Fresnel effect
    float fresnel = pow(1.0 - dot(view_dir, normal), fresnel_power);

    // Combine textures with Fresnel effect
    vec3 finalColor = mix(refraction.rgb, reflection.rgb, fresnel) * sC.rgb;

    // Ensure the alpha does not go below 0.75
    float finalAlpha = max(alpha0, 0.75);

    // Add specular highlight
    vec3 halfVector = normalize(light_dir + view_dir);
    float specular = pow(max(dot(normal, halfVector), 0.0), 32.0); // Shininess factor of 32
    finalColor += vec3(specular);

    outputColor = vec4(finalColor, finalAlpha);
}
