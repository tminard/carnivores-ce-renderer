#version 330

layout(location = 0) in vec3 position;
layout(location = 2) in vec4 texCoords;
layout(location = 3) in float brightness; // 0 = max bright; 55 = max dark

out highp vec2 out_tx1_uv;
out highp vec2 out_tx2_uv;
out lowp float out_fog;
smooth out float out_brightness;

uniform mat4 MVP;
uniform vec3 input_cam_pos;

#define MAX_R_VIEW 76.0
#define TERRAIN_FOG_START 64.0
#define TERRAIN_CELL_SIZE 256.0
#define TERRAIN_SIZE 512.0

const float terrain_fog_k = 1.0 / ((MAX_R_VIEW * TERRAIN_CELL_SIZE) - (TERRAIN_FOG_START * TERRAIN_CELL_SIZE));

void main()
{
    float d = distance(input_cam_pos, position);

    out_fog = (d / TERRAIN_CELL_SIZE) / 4.0;
    out_fog = clamp(out_fog, 0.0, 1.0);

    vec4 v = vec4(position, 1);

    out_tx1_uv = vec2(texCoords[0], texCoords[1]);;
    out_tx2_uv = vec2(texCoords[2], texCoords[3]);
    out_brightness = brightness;

    gl_Position = MVP * v;
}
