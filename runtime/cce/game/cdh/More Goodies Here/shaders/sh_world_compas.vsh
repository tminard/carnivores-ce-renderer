#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#endif

attribute vec3 input_position;
attribute vec2 input_texCoord;

uniform mat4 input_mvp;

//
varying mediump vec2 out_textCoord;



void main()
{
    out_textCoord = input_texCoord * (1.0 / 255.0);
    gl_Position = input_mvp * vec4(input_position.x, input_position.y, input_position.z, 1.0);
}
