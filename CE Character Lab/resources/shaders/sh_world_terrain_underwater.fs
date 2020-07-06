#version 330

in highp vec2 out_tx1_uv;
in highp vec2 out_tx2_uv;
in lowp float out_fog;
smooth in float out_brightness;

out vec4 out_color;

uniform sampler2D basic_texture;

void main()
{
    vec4 c;

    if (mod(gl_PrimitiveID, 2) == 1) {
        c = texture(basic_texture, out_tx1_uv);
    } else {
        c = texture(basic_texture, out_tx2_uv);
    }

    float percent = (out_brightness / 255.0);

    vec3 finalColor = vec3(c.b * percent, c.g * percent, c.r * percent);
    float alpha = out_fog;
    finalColor = finalColor.rgb;

    out_color = vec4(finalColor, alpha);
}
