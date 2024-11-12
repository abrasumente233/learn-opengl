#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aStartColor;
layout (location = 2) in vec3 aEndColor;
layout (location = 3) in vec2 aTexCoord;

out vec3 startColor;
out vec3 endColor;
out vec2 texCoord;

uniform mat4 transform;

void main() {
  startColor = aStartColor;
  endColor = aEndColor;
  texCoord = aTexCoord;
  gl_Position = transform * vec4(aPos, 1.0);
}
