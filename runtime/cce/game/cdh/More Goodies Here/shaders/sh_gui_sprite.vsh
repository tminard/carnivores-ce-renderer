#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#endif

attribute vec2 input_position;
attribute vec2 input_texCoord;
attribute vec4 input_color;

uniform mat4 input_mvp;

varying mediump vec2 out_textCoord;
varying mediump vec4 out_color;

void main()
{
    out_textCoord = input_texCoord;
    out_color = input_color;
    
    gl_Position = input_mvp * vec4(input_position.x, input_position.y, 0.0, 1.0);
}
