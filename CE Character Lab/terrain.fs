#version 330

in vec2 texCoord0;
flat in int texID0;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform int texSquareSize;

void main()
{
    int tex = texID0;
    float tex_size_square = float(texSquareSize);
    int tex_y = int(floor(float(tex)/tex_size_square));
    
    float unit_increase = (1.f / tex_size_square); // map 0-1 to 1/square portion of the image

    // 0-1 maps to a 130x130 block; need to rescale to 128*128
    // float scale_factor_for_clamping = (128.f*tex_size_square)/(130.f*tex_size_square);
    
    float tu = ((tex * unit_increase) + (texCoord0.x * unit_increase));
    float tv = ((tex_y * unit_increase) + (texCoord0.y * unit_increase));
    vec2 UV = vec2(tu, tv);

    vec4 sC = texture(basic_texture, UV);
    vec3 finalColor = vec3(sC.b, sC.g, sC.r);
    finalColor = 1.55 * finalColor.rgb * vec3(0.94, 0.97, 1.0);
   
    outputColor = vec4(finalColor, 1.0);
}
