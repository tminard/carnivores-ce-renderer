#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#endif

attribute vec2 input_position;
attribute vec4 input_color;

uniform mat4 input_mvp;

varying mediump vec4 out_color;

void main()
{
    out_color = input_color;
    gl_Position = input_mvp * vec4(input_position.x, input_position.y, 0.0, 1.0);
}  
/*
  private final String mVertexShaderSimple = "\n" +
            "attribute vec2 aPos;\n" +
            "attribute vec2 aTC;\n" +
            "uniform mat2 uTcmLL;\n" +
            "uniform mat2 uTcmLR;\n" +
            "uniform mat2 uTcmUL;\n" +
            "uniform mat2 uTcmUR;\n" +
            "uniform vec2 uOfsLL;\n" +
            "uniform vec2 uOfsLR;\n" +
            "uniform vec2 uOfsUL;\n" +
            "uniform vec2 uOfsUR;\n" +
            "varying vec2 tc;\n" +
            "varying vec2 tcLL;\n" +
            "varying vec2 tcLR;\n" +
            "varying vec2 tcUL;\n" +
            "varying vec2 tcUR;\n" +
            "void main() {\n" +
            "  tcLL = (aTC - 0.5 + uOfsLL) * uTcmLL;\n" +
            "  tcLL += 0.5;\n" +
            "  tcLR = (aTC - 0.5 + uOfsLR) * uTcmLR;\n" +
            "  tcLR += 0.5;\n" +
            "  tcUL = (aTC - 0.5 + uOfsUL) * uTcmUL;\n" +
            "  tcUL += 0.5;\n" +
            "  tcUR = (aTC - 0.5 + uOfsUR) * uTcmUR;\n" +
            "  tcUR += 0.5;\n" +
            "  tc = aTC;\n" +
            "  gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n" +
            "}";

    private final String mFragmentShaderSimple = "precision mediump float;\n" +
            "uniform sampler2D texLL;\n" +
            "uniform sampler2D texLR;\n" +
            "uniform sampler2D texUL;\n" +
            "uniform sampler2D texUR;\n" +
            "uniform float xK;\n" +
            "uniform float yK;\n" +
            "varying vec2 tc;\n" +
            "varying vec2 tcLL;\n" +
            "varying vec2 tcLR;\n" +
            "varying vec2 tcUL;\n" +
            "varying vec2 tcUR;\n" +
            "void main() {\n" +
            "  vec4 colorL = texture2D(texLL, tcLL) * (1.0 - xK) + texture2D(texLR, tcLR) * (xK);\n" +
            "  vec4 colorU = texture2D(texUL, tcUL) * (1.0 - xK) + texture2D(texUR, tcUR) * (xK);\n" +
            "  gl_FragColor = colorL * (1.0 - yK) + colorU * (yK);\n" +
            "}";

    private final String mFragmentShaderInterlace = "precision mediump float;\n" +
            "uniform sampler2D texLL;\n" +
            "uniform sampler2D texLR;\n" +
            "uniform sampler2D texUL;\n" +
            "uniform sampler2D texUR;\n" +
            "uniform sampler2D texLut;\n" +
            "uniform float xK;\n" +
            "uniform float yK;\n" +
            "varying vec2 tc;\n" +
            "varying vec2 tcLL;\n" +
            "varying vec2 tcLR;\n" +
            "varying vec2 tcUL;\n" +
            "varying vec2 tcUR;\n" +
            "uniform float xShift;\n" +
            "uniform float brightness;\n" +
            "uniform float contrast;\n" +
            "uniform float saturation;\n" +
            "const float lensCount = 240.0;\n" +
            "vec3 rgb2hsv(vec3 c)\n" +
            "{\n" +
            "    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n" +
            "    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n" +
            "    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n" +
            "\n" +
            "    float d = q.x - min(q.w, q.y);\n" +
            "    float e = 1.0e-10;\n" +
            "    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n" +
            "}\n" +
            "vec3 hsv2rgb(vec3 c)\n" +
            "{\n" +
            "    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n" +
            "    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n" +
            "    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n" +
            "}\n" +
            "void main() {\n" +
            "  // Blend\n" +
            "  float lens_kX = (tc.s - 0.5) * 0.35;\n" +
            "  lens_kX *= abs(sin((tc.s * lensCount) * 3.1416 * 1.0));\n" +
            "  lens_kX += xK;\n" +
            "  lens_kX = clamp(lens_kX, 0.0, 1.0);\n" +
            "  float lens_kY = (tc.t - 0.5) * 0.35;\n" +
            "  lens_kY *= abs(sin((tc.t * lensCount) * 3.1416 * 1.0));\n" +
            "  lens_kY += yK;\n" +
            "  lens_kY = clamp(lens_kY, 0.0, 1.0);\n" +

            "  vec2 ltcLL = vec2(tcLL.s + (xShift * 2.0 / lensCount), tcLL.t);\n" +
            "  vec2 ltcLR = vec2(tcLR.s + (xShift * 2.0 / lensCount), tcLR.t);\n" +
            "  vec2 ltcUL = vec2(tcUL.s + (xShift * 2.0 / lensCount), tcUL.t);\n" +
            "  vec2 ltcUR = vec2(tcUR.s + (xShift * 2.0 / lensCount), tcUR.t);\n" +
            "  vec4 colorL = texture2D(texLL, ltcLL) * (1.0 - lens_kX) + texture2D(texLR, ltcLR) * (lens_kX);\n" +
            "  vec4 colorU = texture2D(texUL, ltcUL) * (1.0 - lens_kX) + texture2D(texUR, ltcUR) * (lens_kX);\n" +
            "  vec4 color = colorL * (1.0 - lens_kY) + colorU * (lens_kY);\n" +

            /*"  // color correction \n" +
            "  vec3 hsv = rgb2hsv(color.rgb);\n" +
            "  hsv.b *= brightness;\n" +
            "  hsv.g *= saturation;\n" +
            "  hsv = clamp(hsv, 0.0, 1.0);\n" +
            "  color.rgb = hsv2rgb(hsv);\n" +*/

            "  color.rgb = (color.rgb - 0.5) * contrast + 0.5;\n" +
            "  color.rgb += brightness;\n" +
            "  float l = dot(color.rgb, vec3(0.299, 0.587, 0.114));\n" +
            "  color.rgb = mix(vec3(l, l, l), color.rgb, saturation);\n" +
            "  color.rgb = clamp(color.rgb, 0.0, 1.0);\n" +

            "  // LUT \n" +
            "  vec2 tcLut = vec2(floor(color.b * 15.0) / 16.0 + (color.r * 15.0) / 256.0, (color.g * 15.0) / 16.0);\n" +
            "  tcLut.s += 0.5 / 256.0;\n" +
            "  tcLut.t += 0.5 / 16.0;\n" +
            "  float bK = (color.b * 15.0) - floor(color.b * 15.0);\n" +
            "  color.rgb = mix(texture2D(texLut, tcLut).rgb, texture2D(texLut, vec2(tcLut.s + (1.0 / 16.0), tcLut.t)).rgb, bK);\n" +

            "  gl_FragColor = color;\n" +
            " // Specular\n" +
            "  gl_FragColor.rgb += abs(lens_kX - 0.5) * 0.1;\n" +
            "  //gl_FragColor.rgb += abs(lens_kY - 0.5) * 0.1;\n" +
            "}";
*/
