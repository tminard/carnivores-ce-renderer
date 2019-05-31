#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#endif

attribute vec3 input_position;
attribute vec2 input_texCoord;
attribute vec4 input_color;

uniform mat4 input_mvp;
uniform vec2 input_tex_scale;
uniform vec4 input_ambient_color;

varying mediump vec2 out_textCoord;
varying mediump vec4 out_color;

void main()
{
    out_textCoord = input_texCoord * input_tex_scale;
    out_color = (input_color / 255.0) * input_ambient_color;
    
    gl_Position = input_mvp * vec4(input_position.x, input_position.y, input_position.z, 1.0);
}
