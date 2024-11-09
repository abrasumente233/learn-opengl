#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 ourColor;
out vec3 ourPos;

uniform float offsetX;

void main() {
  ourColor = aColor;
  ourPos = aPos;
  gl_Position = vec4(aPos.x + offsetX, aPos.y, aPos.z, 1.0);
}
