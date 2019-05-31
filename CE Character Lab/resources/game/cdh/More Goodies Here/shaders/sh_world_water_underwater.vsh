#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#endif

attribute highp vec3 input_position; // TODO: check if could use lower
attribute mediump vec4 input_color;

uniform highp mat4 input_mvp;
uniform vec3 input_cam_pos;
uniform mediump vec2 input_fog_params;

varying mediump vec3 out_params; // r - reflection value, g - distant fog, b - waves

#define pi 3.141592653589793238462643383279
#define MAX_R_VIEW 76.0
#define TERRAIN_FOG_START 64.0
#define TERRAIN_CELL_SIZE 256.0

const float terrain_fog_k = 1.0 / ((MAX_R_VIEW * TERRAIN_CELL_SIZE) - (TERRAIN_FOG_START * TERRAIN_CELL_SIZE));

void main()
{
	// waves
	out_params.b = input_color.a / 255.0;
    
	// calculating distance for fogs
	float vertex_dist = distance(input_cam_pos, input_position);
	
	// distant fog
	out_params.g = clamp((1.0 - (vertex_dist - input_fog_params.x) / (input_fog_params.y - input_fog_params.x)), 0.0, 1.0);
	
	out_params.r = clamp(out_params.g * ((input_color.a / 32.0) + 0.2), 0.0, 1.0);
	
	// vertex position
    gl_Position = input_mvp * vec4(input_position.x, input_position.y, input_position.z, 1.0);
}
