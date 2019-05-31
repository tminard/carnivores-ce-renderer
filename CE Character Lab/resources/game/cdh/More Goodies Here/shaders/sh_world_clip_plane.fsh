#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#else
  precision mediump float;
#endif

void main()
{
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
	//gl_FragDepth = 1.0;
}
