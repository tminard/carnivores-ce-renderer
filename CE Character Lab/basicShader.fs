#version 330

in vec2 texCoord0;

out vec4 outputColor;

uniform sampler2D basic_texture;

void main()
{
  vec4 sC = texture( basic_texture, texCoord0 );
  float trans = 0.095;
  
  if (sC.r <= trans && sC.g <= trans && sC.b <= trans) {
    discard;
  }

  outputColor = vec4(sC.b, sC.g, sC.r, 1.0f);
}
