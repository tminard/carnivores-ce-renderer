#version 330

in vec2 texCoord0;
flat in int texID0;
in vec3 worldPos;
in vec4 viewSpace;
in float distanceToCamera;

out vec4 outputColor;

uniform sampler2D basic_texture;

//const vec3 fogColor = vec3(0.94, 0.97, 1.0);
const int FOG_START = 256*48;
const int FOG_END = 256*49;
//const int MAX_RANGE = 256*50;

void main()
{
    float alpha = 1.0;
    float dist = abs(distanceToCamera);
    int tex = texID0; // get tile´s texture ID
    float tex_size_square = 8.f; // TODO: get this from the map rsc file - textures are stored in a single atlas
    int tex_y = int(floor(float(tex)/tex_size_square));
    
    float unit_increase = (1.f / tex_size_square); // map 0-1 to 1/square portion of the image

    // 0-1 maps to a 130x130 block; need to rescale to 128*128
    //float scale_factor_for_clamping = (128.f*tex_size_square)/(130.f*tex_size_square);
    
    float tu = ((tex * unit_increase) + (texCoord0.x * unit_increase));
    float tv = ((tex_y * unit_increase) + (texCoord0.y * unit_increase));
    vec2 UV = vec2(tu, tv);

    vec4 sC = texture(basic_texture, UV);
    
    float fogFactor = (FOG_END - dist)/(FOG_END - FOG_START);
    vec3 finalColor;
    
    vec3 lightColor = vec3(sC.b, sC.g, sC.r);
    lightColor = 1.55 * lightColor.rgb * vec3(0.94, 0.97, 1.0);
    
    //fogFactor = clamp(fogFactor, 0.0, 1.0);
    float distToFade = (dist - FOG_START); //1-(6-5)/(10-5)
    if (distToFade > 0) {
        alpha = 1.f - (distToFade / float(FOG_END - FOG_START));
    }
    
    //finalColor = mix(fogColor, lightColor, fogFactor);
   
    outputColor = vec4(lightColor, alpha);
}
