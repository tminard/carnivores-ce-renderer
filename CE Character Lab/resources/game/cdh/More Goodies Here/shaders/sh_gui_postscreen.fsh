#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#else
  precision mediump float;
#endif

varying vec4 out_color;

void main()
{
/*            "  vec2 tcLut = vec2(floor(color.b * 15.0) / 16.0 + (color.r * 15.0) / 256.0, (color.g * 15.0) / 16.0);\n" +
            "  tcLut.s += 0.5 / 256.0;\n" +
            "  tcLut.t += 0.5 / 16.0;\n" +
            "  float bK = (color.b * 15.0) - floor(color.b * 15.0);\n" +
            "  color.rgb = mix(texture2D(texLut, tcLut).rgb, texture2D(texLut, vec2(tcLut.s + (1.0 / 16.0), tcLut.t)).rgb, bK);\n" +
    gl_FragColor = out_color / 255.0;*/
}
