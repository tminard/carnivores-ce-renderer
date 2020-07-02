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

uniform sampler2D input_texture;
uniform vec3 input_sky_color;


void main()
{
    vec4 base_color = texture2D(input_texture, out_textCoord);
    
	vec3 color = base_color.rgb * out_color.rgb;
	
	// calculating distant fog
	color = mix(input_sky_color, color, out_distant_fog);
    
    if (out_water_clipping > 0.0)
	{
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
	}
	/*
    if (base_color.a < 0.6)
    {
        gl_FragColor = vec4(color, min(out_color.a, base_color.a));
    }
	*/
    else
    {
        gl_FragColor = vec4(color, base_color.a);
    }
}
