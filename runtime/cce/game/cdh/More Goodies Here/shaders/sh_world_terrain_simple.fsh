#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#else
  precision mediump float;
#endif

varying lowp vec2 out_color;
varying vec2 out_textCoord;
varying vec2 out_textCoord_shadowmap;
varying vec2 out_textCoord_clouds;

uniform sampler2D input_texture;
uniform sampler2D input_texture_shadowmap;
uniform sampler2D input_texture_clouds;
uniform vec3 input_light_color;
uniform vec3 input_shadows_color;
uniform vec3 input_sky_color;


void main()
{
	lowp float shadow_color = texture2D(input_texture_shadowmap, out_textCoord_shadowmap).r * texture2D(input_texture_clouds, out_textCoord_clouds).r;
	
	lowp vec3 diffuse = (input_light_color * shadow_color + input_shadows_color) * (texture2D(input_texture, out_textCoord).rgb);
	
	diffuse = mix(input_sky_color, diffuse, out_color.g);
	
	gl_FragColor = vec4(diffuse, out_color.r);
}
