#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#else
  precision mediump float;
#endif

varying vec4 out_color;
varying vec2 out_textCoord;
varying float out_distant_fog;
// varying float out_water_clipping;

uniform sampler2D input_texture;
uniform vec3 input_sky_color;


void main()
{
    vec4 base_color = texture2D(input_texture, out_textCoord);

	lowp float alphaCutoff = sign(max(0.6, base_color.a) - 0.6);
    
	vec3 color = base_color.rgb * out_color.rgb;
	
	// calculating distant fog
	color = mix(input_sky_color, color, out_distant_fog);
    
    gl_FragColor = vec4(color, alphaCutoff * base_color.a);
}
