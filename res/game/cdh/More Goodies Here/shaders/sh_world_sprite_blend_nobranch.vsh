#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#endif

attribute vec3 input_position;
attribute vec2 input_texCoord;
attribute float input_color;

uniform mat4 input_mvp;
uniform vec3 input_cam_pos;
uniform vec2 input_tex_scale;
uniform vec3 input_light_color;
uniform vec3 input_shadows_color;
uniform float input_water_level;
uniform mediump vec2 input_fog_params;

varying mediump vec2 out_textCoord;
varying mediump vec4 out_color;
varying mediump float out_distant_fog;
// varying mediump float out_water_clipping;


#define MAX_R_VIEW 76.0
#define TERRAIN_FOG_START 64.0
#define TERRAIN_COLOR_FOG_START 38.0
#define TERRAIN_CELL_SIZE 256.0
#define TERRAIN_SIZE 1024.0

const float terrain_fog_k = 1.0 / ((MAX_R_VIEW * TERRAIN_CELL_SIZE) - (TERRAIN_FOG_START * TERRAIN_CELL_SIZE));
const float terrain_color_fog_k = 1.0 / ((MAX_R_VIEW * TERRAIN_CELL_SIZE) - (TERRAIN_COLOR_FOG_START * TERRAIN_CELL_SIZE));

void main()
{
    out_textCoord = input_texCoord * input_tex_scale;
		
	// calculating distance for fogs
	float vertex_dist = distance(input_cam_pos, input_position);
	
	// distant fog
	out_distant_fog = clamp((1.0 - (vertex_dist - input_fog_params.x) / (input_fog_params.y - input_fog_params.x)), 0.0, 1.0);
	
	// transparent fog
	float a = 1.0 - (vertex_dist - (TERRAIN_FOG_START * TERRAIN_CELL_SIZE)) * terrain_fog_k;
	a = clamp(a, 0.0, 1.0);
	
	// color
	out_color = vec4((input_light_color * (input_color / 255.0) + input_shadows_color), a);
	
	// water clipping
//	out_water_clipping = input_water_level - input_position.y;
    
    gl_Position = input_mvp * vec4(input_position.x, input_position.y, input_position.z, 1.0);
}
