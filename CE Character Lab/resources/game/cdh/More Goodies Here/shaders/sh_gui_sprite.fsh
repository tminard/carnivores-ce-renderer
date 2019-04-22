#ifndef GL_ES
  #define lowp
  #define mediump
  #define highp
#else
  precision mediump float;
#endif

varying vec4 out_color;
varying vec2 out_textCoord;

uniform sampler2D input_texture;

void main()
{
    vec4 resultColor = texture2D(input_texture, out_textCoord);
    
    resultColor.x *= out_color.x / 255.0;
    resultColor.y *= out_color.y / 255.0;
    resultColor.z *= out_color.z / 255.0;
    resultColor.w *= out_color.w / 255.0;
    
    gl_FragColor = resultColor;
}
