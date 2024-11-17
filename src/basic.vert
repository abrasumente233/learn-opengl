#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 normal;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  normal = aNormal;
  vec4 viewPos = view * model * vec4(aPos, 1.0);
  fragPos = viewPos.xyz;
  gl_Position = projection * viewPos;
}
