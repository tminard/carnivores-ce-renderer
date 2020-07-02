#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#else
  precision mediump float;
#endif

varying lowp vec3 out_color;
varying vec2 out_textCoord;
varying vec2 out_textCoord_shadowmap;
varying highp vec2 out_textCoord_clouds;
varying vec2 out_textCoord_shadows;
varying vec2 out_textCoord_detail;

uniform sampler2D input_texture;
uniform sampler2D input_texture_shadowmap;
uniform sampler2D input_texture_clouds;
uniform sampler2D input_texture_detail;
uniform sampler2D input_texture_shadows;
uniform vec3 input_light_color;
uniform vec3 input_shadows_color;
uniform vec3 input_sky_color;


void main()
{
	lowp float detail_color = (texture2D(input_texture_detail, out_textCoord_detail).r - 0.5) * out_color.b;

	lowp float shadowmap = texture2D(input_texture_shadowmap, out_textCoord_shadowmap).r * texture2D(input_texture_clouds, out_textCoord_clouds).r;
	lowp float shadow = 1.0 - texture2D(input_texture_shadows, out_textCoord_shadows).r;
	lowp float shadow_color = shadow * shadowmap;
	
	lowp vec3 diffuse = (input_light_color * shadow_color + input_shadows_color) * (texture2D(input_texture, out_textCoord).rgb + detail_color);
	
	diffuse = mix(input_sky_color, diffuse, out_color.g);
	
	//
	
	gl_FragColor = vec4(diffuse, out_color.r);
}
