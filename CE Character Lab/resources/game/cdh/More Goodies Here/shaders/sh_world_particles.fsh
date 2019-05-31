#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#else
  precision mediump float;
#endif

varying vec4 out_color;
varying vec2 out_textCoord;

uniform sampler2D input_texture;

void main()
{
    vec4 diffuse = texture2D(input_texture, out_textCoord);
    gl_FragColor = vec4(diffuse.xyz * out_color.xyz, diffuse.a * out_color.w);
}
