#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 3) in int texID;

out vec2 texCoord0;
flat out int texID0;
out vec3 worldPos;
out vec4 viewSpace;

uniform mat4 MVP;

void main()
{
  vec4 v = vec4(position, 1);
  gl_Position = MVP * v;
  texCoord0 = texCoord;
  texID0 = texID;
  worldPos = (MVP * v).xyz;
  viewSpace = MVP * v;
}
