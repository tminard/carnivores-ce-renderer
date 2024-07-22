#version 330 core
out vec4 outputColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform vec4 sky_color;
uniform float time; // Pass the elapsed time from your application

// Simple pseudo-random function
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Smooth random function
float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
           (c - a) * u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

void main()
{
    // Generate noise based on the TexCoords and time
    //float n = noise(TexCoords.xy + time * 0.1);

    // Modulate the sky color with the noise to create a glimmer effect
    //vec4 dynamicSkyColor = sky_color + vec4(vec3(n) * 0.05, 0.0);

    // Sample the skybox texture
    //vec4 sampledSkybox = texture(skybox, TexCoords);

    // Combine the dynamic sky color with the sampled skybox texture, favoring the sky color more
    // outputColor = mix(dynamicSkyColor, sampledSkybox, 0.05); // Adjusted mix factor for stronger sky color influence
    outputColor = sky_color;
}
