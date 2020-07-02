#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#else
  precision mediump float;
#endif

varying vec4 out_color;

void main()
{
    gl_FragColor = out_color / 255.0;
}
