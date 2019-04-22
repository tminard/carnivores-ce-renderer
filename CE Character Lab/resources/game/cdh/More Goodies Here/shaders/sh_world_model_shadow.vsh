#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#endif

attribute vec3 input_position;
attribute vec2 input_texCoord;

uniform mat4 input_mvp;
uniform vec3 input_cam_pos;  // TODO: calculate on the fly
uniform vec2 input_tex_scale;

varying lowp float out_fade;
varying mediump vec2 out_textCoord;

const float fade_dist = 4096.0;

void main()
{
    out_textCoord = input_texCoord * input_tex_scale;

    gl_Position = input_mvp * vec4(input_position.x, input_position.y, input_position.z, 1.0);
	
	float vertex_dist = length(gl_Position.xy);
	out_fade = 1.0 - vertex_dist;
	out_fade = clamp(out_fade, 0.0, 0.4);
	
}
