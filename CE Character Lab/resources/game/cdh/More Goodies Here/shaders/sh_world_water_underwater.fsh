#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#else
  precision mediump float;
#endif

varying mediump vec3 out_params;
varying lowp vec3 out_color;

uniform sampler2D input_texture_reflection;
uniform vec2 input_resolution;
uniform vec3 input_sky_color;


void main()
{
	// wave
	mediump float wave = out_params.b;
	
	// calculating reflection color
	vec2 reflection_tc = (gl_FragCoord.xy / input_resolution) + wave;
	
	float shift = (1.0 / 512.0) * (2.0 + wave * 2.0);
	
	vec4 reflection_color1 = texture2D(input_texture_reflection, vec2(reflection_tc.x + shift, reflection_tc.y));
	vec4 reflection_color2 = texture2D(input_texture_reflection, vec2(reflection_tc.x - shift, reflection_tc.y));
	vec4 reflection_color3 = texture2D(input_texture_reflection, vec2(reflection_tc.x, reflection_tc.y + shift));
	vec4 reflection_color4 = texture2D(input_texture_reflection, vec2(reflection_tc.x, reflection_tc.y - shift));
	
	vec4 reflection_color = (reflection_color1 + reflection_color2 + reflection_color3 + reflection_color4) * 0.25;
	//vec4 reflection_color = texture2D(input_texture_reflection, vec2((gl_FragCoord.x / input_resolution.x) + wave , (gl_FragCoord.y / input_resolution.y) + wave));
	reflection_color.rgb += out_params.b * 3.0;
	
	reflection_color.rgb = clamp(reflection_color.rgb, 0.0, 1.0);
		
	// mixing it to final color
	vec3 diffuse = mix(input_sky_color, reflection_color.rgb, out_params.r);
	
	// calculating distant fog
	//diffuse = mix(input_sky_color, diffuse, out_params.g);
	
	gl_FragColor = vec4(diffuse, 1.0);
}
