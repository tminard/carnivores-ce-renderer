#version 330

in vec2 texCoord0;

out vec4 outputColor;

uniform sampler2D basic_texture;

void main()
{
  int tex = 19; // get tile´s texture ID
  int tex_y = int(floor(float(tex)/8.f));

  vec2 UV = vec2((1.f/8.f) * (tex + fract(texCoord0.x/3.f)), (1.f/8.f) * (tex_y + fract(texCoord0.y/3.f)));

  vec4 sC = texture( basic_texture, UV );
  outputColor = vec4(sC.b, sC.g, sC.r, 1.0);
}