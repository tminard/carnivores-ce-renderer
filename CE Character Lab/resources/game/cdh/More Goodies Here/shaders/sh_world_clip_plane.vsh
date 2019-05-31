#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#endif

attribute vec3 input_position;
uniform mat4 input_mvp;

void main()
{
    gl_Position = input_mvp * vec4(input_position.x, input_position.y, input_position.z, 1.0);
	//gl_Position.w = 1.0;
	//gl_Position.z = 1.0;
}
