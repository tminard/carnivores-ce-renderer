#version 330

in vec2 texCoord0;

out vec4 outputColor;

uniform sampler2D basic_texture;

void main()
{
  vec4 sC = texture( basic_texture, texCoord0 );
  float al = 1.0;
  
  if (sC.r <= 0.025 && sC.g <= 0.025 && sC.b <= 0.025) {
    al = 0.0;
  }
  outputColor = vec4(sC.b, sC.g, sC.r, al);
}