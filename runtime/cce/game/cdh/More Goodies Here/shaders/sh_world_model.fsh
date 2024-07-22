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
varying float out_water_clipping;
varying float out_detail;

uniform sampler2D input_texture;
uniform sampler2D input_texture_detail;
uniform vec3 input_sky_color;

void main()
{
	vec4 base_color = texture2D(input_texture, out_textCoord);
	
    if (base_color.a < 0.6 || out_water_clipping > 0.0)
    {
        discard;
    }
	
	float detail_color = (texture2D(input_texture_detail, out_textCoord * 8.0).r - 0.5) * out_detail;
	
	vec3 color = (base_color.rgb + detail_color) * out_color.rgb;
	
	// calculating distant fog
	color = mix(input_sky_color, color, out_distant_fog);
	
	
    gl_FragColor = vec4(color, out_color.a);
}
