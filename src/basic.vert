#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aStartColor;
layout (location = 2) in vec3 aEndColor;
out vec3 startColor;
out vec3 endColor;

void main() {
  startColor = aStartColor;
  endColor = aEndColor;
  gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
