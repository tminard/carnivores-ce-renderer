#version 330

in vec2 texCoord0;

out vec4 outputColor;

uniform sampler2D basic_texture;

void main()
{
  vec4 sC = texture( basic_texture, texCoord0 );
  outputColor = vec4(sC.b, sC.g, sC.r, 1.0);
}