#version 330

const vec3 DiffuseLight = vec3(0.15, 0.05, 0.0);
const vec3 RimColor = vec3(0.2, 0.2, 0.2);

in vec2 texCoord0;
flat in int texID0;
in vec3 worldPos;
in vec4 viewSpace;

out vec4 outputColor;

uniform sampler2D basic_texture;

const vec3 fogColor = vec3(0.5, 0.5,0.5);
const float FogDensity = 0.05;
const int FOG_START = 12000;
const int FOG_END = 18000;

void main()
{
  int tex = texID0; // get tile´s texture ID
  float tex_size_square = 8.f; //TODO: get this from the map rsc file - textures are stored in a single atlas
  int tex_y = int(floor(float(tex)/tex_size_square));

  float unit_increase = (1.f / tex_size_square);

  vec2 UV = vec2(((tex * unit_increase) + (texCoord0.x * unit_increase)), ((tex_y * unit_increase) + (texCoord0.y * unit_increase)));
  vec4 sC = texture( basic_texture, UV );

  float dist = length(worldPos);
  float fogFactor = (FOG_END - dist)/(FOG_END - FOG_START);
  vec3 finalColor;

  vec3 lightColor = vec3(sC.b, sC.g, sC.r);

  fogFactor = clamp( fogFactor, 0.0, 1.0 );

  finalColor = mix(fogColor, lightColor, fogFactor);

  outputColor = vec4(finalColor, 1.0);
}
