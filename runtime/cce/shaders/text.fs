#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D fontTexture; // Bitmap font texture atlas
uniform vec3 textColor;        // Text color
uniform float textAlpha;       // Text transparency

void main()
{
    // Sample the font texture (alpha channel contains the character shape)
    vec4 texSample = texture(fontTexture, TexCoord);
    
    // For bitmap fonts, we typically use the red channel or alpha channel for the text
    float alpha = texSample.r; // Assuming red channel contains the text data
    
    // Discard fully transparent pixels
    if (alpha < 0.01) {
        discard;
    }
    
    // Output the text color with the font alpha
    FragColor = vec4(textColor, alpha * textAlpha);
}