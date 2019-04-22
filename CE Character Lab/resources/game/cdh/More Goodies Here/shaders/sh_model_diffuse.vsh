#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#endif

attribute vec3 input_position;
attribute vec2 input_texCoord;

uniform mat4 input_mvp;
uniform vec2 input_tex_scale;

varying mediump vec2 out_textCoord;


void main()
{
    out_textCoord = input_texCoord * input_tex_scale;
    gl_Position = input_mvp * vec4(input_position.x, input_position.y, input_position.z, 1.0);
}
