#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#else
  precision mediump float;
#endif

varying lowp vec2 out_fog;
varying mediump float out_water_clipping;
varying vec4 out_tc;
varying vec2 out_tc_shadowmap;
varying highp vec2 out_tc_clouds;
varying vec2 out_tc_shadows;

uniform sampler2D input_texture_tiles;
uniform sampler2D input_texture_grass;
uniform sampler2D input_texture_shadowmap;
uniform sampler2D input_texture_clouds;
uniform sampler2D input_texture_shadows;
uniform vec3 input_light_color;
uniform vec3 input_shadows_color;
uniform vec3 input_sky_color;


void main()
{
    vec4 base_color = texture2D(input_texture_grass, vec2(out_tc.x, out_tc.y));
    
    lowp float shadowmap = texture2D(input_texture_shadowmap, out_tc_shadowmap).r * texture2D(input_texture_clouds, out_tc_clouds).r;
    lowp float shadow = 1.0 - texture2D(input_texture_shadows, out_tc_shadows).r;
    lowp float shadow_color = shadow * shadowmap;
    
    lowp vec3 diffuse = (base_color.rgb * texture2D(input_texture_tiles, vec2(out_tc.z, out_tc.w)).rgb * 2.0) * (input_light_color * shadow_color + input_shadows_color);
    
    diffuse = mix(input_sky_color, diffuse, out_fog.g);
    
    if (out_water_clipping > 0.0 || base_color.a < 0.6)
	{
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
	}
    else
    {
        gl_FragColor = vec4(diffuse, out_fog.r);
    }
}
