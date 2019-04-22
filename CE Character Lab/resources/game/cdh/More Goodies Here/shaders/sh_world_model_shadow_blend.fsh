#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#else
  precision mediump float;
#endif

varying lowp float out_fade;
varying mediump vec2 out_textCoord;

uniform sampler2D input_texture;
uniform float input_shadow;

void main()
{
	lowp float alpha = texture2D(input_texture, out_textCoord).a;

	lowp float alphaCutoff = sign(max(0.6, alpha) - 0.6);
	
    gl_FragColor = vec4(out_fade * input_shadow, 1.0, 1.0, alphaCutoff);
}
