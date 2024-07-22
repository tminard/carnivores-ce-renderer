#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#else
  precision mediump float;
#endif

varying lowp float out_fog;

void main()
{
	gl_FragColor = vec4(out_fog, 1.0, 1.0, 1.0);
}
