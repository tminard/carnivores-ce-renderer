#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#endif

attribute highp vec3 input_position;

uniform highp mat4 input_mvp;
uniform vec3 input_cam_pos;
uniform mediump float input_water_level;

varying lowp float out_fog;

#define MAX_R_VIEW 76.0
#define TERRAIN_FOG_START 64.0
#define TERRAIN_CELL_SIZE 256.0
#define TERRAIN_SIZE 1024.0


void main()
{
	float vertex_dist = distance(input_cam_pos, input_position);
	
	float water_dist_k = (input_water_level - input_position.y) / (input_cam_pos.y - input_position.y);
	
	float d = vertex_dist * water_dist_k;
		
	out_fog = (d / TERRAIN_CELL_SIZE) / 4.0;
	out_fog = clamp(out_fog, 0.0, 1.0);

	
    gl_Position = input_mvp * vec4(input_position.x, input_position.y, input_position.z, 1.0);
}
