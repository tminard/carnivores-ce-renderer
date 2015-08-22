#version 330

in vec2 texCoord0;

out vec4 outputColor;

uniform sampler2D basic_texture;

void main()
{
  outputColor = texture( basic_texture, texCoord0 );
}