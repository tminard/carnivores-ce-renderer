#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#endif

attribute highp vec3 input_position;
attribute vec2 input_texCoord;

uniform highp mat4 input_mvp;
uniform vec2 input_tex_scale;
uniform vec3 input_cam_pos;
uniform highp float input_time;
uniform mediump vec2 input_fog_params;

varying mediump vec2 out_textCoord;
varying mediump vec2 out_textCoord_shadowmap;
varying highp vec2 out_textCoord_clouds;
varying mediump vec2 out_textCoord_shadows;
varying mediump vec2 out_textCoord_detail;
varying lowp vec3 out_color;

#define MAX_R_VIEW 76.0
#define TERRAIN_FOG_START 64.0
#define TERRAIN_CELL_SIZE 256.0
#define TERRAIN_SIZE 1024.0

const float terrain_fog_k = 1.0 / ((MAX_R_VIEW * TERRAIN_CELL_SIZE) - (TERRAIN_FOG_START * TERRAIN_CELL_SIZE));
const float detailmap_dist = 1024.0;

void main()
{
	float vertex_dist = distance(input_cam_pos, input_position); // TODO: could use it in depth as alpha

	// transparent fog
	out_color.r = 1.0 - (vertex_dist - (TERRAIN_FOG_START * TERRAIN_CELL_SIZE)) * terrain_fog_k;
	out_color.r = clamp(out_color.r, 0.0, 1.0);
	
	// distant fog
	out_color.g = clamp((1.0 - (vertex_dist - input_fog_params.x) / (input_fog_params.y - input_fog_params.x)), 0.0, 1.0);
	
	// detailmap
	out_color.b = 1.0 - (abs(vertex_dist - detailmap_dist) / detailmap_dist);
	out_color.b = clamp(out_color.b, 0.0, 1.0);

    out_textCoord = input_texCoord * input_tex_scale;
	
	out_textCoord_shadowmap = vec2(-input_position.z / (TERRAIN_CELL_SIZE * TERRAIN_SIZE), -input_position.x / (TERRAIN_CELL_SIZE * TERRAIN_SIZE));
	out_textCoord_clouds = vec2(1.0 - (input_position.z / (TERRAIN_CELL_SIZE * 128.0)) - input_time, 1.0 - (input_position.x / (TERRAIN_CELL_SIZE * 128.0)) - input_time);
	
	out_textCoord_detail = vec2(-input_position.z / TERRAIN_CELL_SIZE, -input_position.x / TERRAIN_CELL_SIZE);
	
	out_textCoord_shadows = vec2(0.5 - ((input_cam_pos.x - input_position.x) / (8192.0 * 2.0)), 0.5 - (-(input_cam_pos.z - input_position.z) / (8192.0 * 2.0)));

    gl_Position = input_mvp * vec4(input_position.x, input_position.y, input_position.z, 1.0);
}
