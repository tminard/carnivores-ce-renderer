#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#endif

attribute vec3 input_position;
attribute vec2 input_texCoord;
attribute float input_shades;

uniform mat4 input_mvp;
uniform vec3 input_cam_pos;
uniform vec2 input_tex_scale;
uniform float input_water_level;
uniform vec4 input_color;
uniform mediump vec2 input_fog_params;

varying vec4 out_color;
varying mediump vec2 out_textCoord;
varying mediump float out_distant_fog;
varying mediump float out_water_clipping;
varying mediump float out_detail;

const float detailmap_dist = 1024.0;

void main()
{
	// color
	out_color = vec4(input_color.rgb * (input_shades / 255.0), input_color.a);
	
    out_textCoord = input_texCoord * input_tex_scale;

    
    gl_Position = input_mvp * vec4(input_position.x, input_position.y, input_position.z, 1.0);
	
	// calculating distance for fogs
	float vertex_dist = gl_Position.z;//distance(input_cam_pos, input_position); // TODO: use this in other places too
	
	// distant fog
	out_distant_fog = clamp((1.0 - (vertex_dist - input_fog_params.x) / (input_fog_params.y - input_fog_params.x)), 0.0, 1.0);
	
	// detailmap
	out_detail = 1.0 - (abs(vertex_dist - detailmap_dist) / detailmap_dist);
	out_detail = clamp(out_detail, 0.0, 1.0);
	
	// water clipping
	out_water_clipping = input_water_level - input_position.y;
}
