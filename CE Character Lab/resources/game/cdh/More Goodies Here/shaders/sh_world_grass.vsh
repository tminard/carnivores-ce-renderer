#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#endif

attribute highp vec3 input_position;
attribute vec4 input_tc;
attribute vec3 input_params;

uniform highp mat4 input_mvp;
uniform vec3 input_cam_pos;
uniform highp vec2 input_time;
uniform mediump vec2 input_fog_params;
uniform float input_water_level;
uniform highp vec4 input_wind_params;

varying lowp vec2 out_fog;
varying mediump float out_water_clipping;
varying vec4 out_tc;
varying vec2 out_tc_shadowmap;
varying highp vec2 out_tc_clouds;
varying vec2 out_tc_shadows;

#define MAX_R_VIEW 76.0
#define TERRAIN_FOG_START 64.0
#define TERRAIN_CELL_SIZE 256.0
#define TERRAIN_SIZE 1024.0

const float terrain_fog_k = 1.0 / ((MAX_R_VIEW * TERRAIN_CELL_SIZE) - (TERRAIN_FOG_START * TERRAIN_CELL_SIZE));
const float detailmap_dist = 1024.0;

void main()
{
	float vertex_dist = distance(input_cam_pos, input_position);
	
	// wind
	float xk = sin((input_time.x + input_params.g) * 4.0) * (2.0 / 128.0);
	float yk = cos((input_time.x + input_params.g) * 3.0) * (1.0 / 128.0);
	
	vec3 position = input_position;
	
	position.x += (xk + yk) * input_params.r;
	position.y += yk * input_params.r;
	
	// transparent fog
	out_fog.r = 1.0 - (vertex_dist - (TERRAIN_FOG_START * TERRAIN_CELL_SIZE)) * terrain_fog_k; // TODO: no need
	out_fog.r = clamp(out_fog.r, 0.0, 1.0);
	
	// distant fog
	out_fog.g = clamp((1.0 - (vertex_dist - input_fog_params.x) / (input_fog_params.y - input_fog_params.x)), 0.0, 1.0);
	
    out_tc = input_tc;
	
	out_tc_shadowmap = vec2(-position.z / (TERRAIN_CELL_SIZE * TERRAIN_SIZE), -position.x / (TERRAIN_CELL_SIZE * TERRAIN_SIZE));
	out_tc_clouds = vec2(1.0 - (position.z / (TERRAIN_CELL_SIZE * 128.0)) - input_time.y, 1.0 - (position.x / (TERRAIN_CELL_SIZE * 128.0)) - input_time.y);
	out_tc_shadows = vec2(0.5 - ((input_cam_pos.x - position.x) / (8192.0 * 2.0)), 0.5 - (-(input_cam_pos.z - position.z) / (8192.0 * 2.0)));
	
	// water clipping
	out_water_clipping = input_water_level - position.y;
	
	// alpha
	out_fog.r *= input_params.b;
	
    gl_Position = input_mvp * vec4(position.x, position.y, position.z, 1.0);
}
