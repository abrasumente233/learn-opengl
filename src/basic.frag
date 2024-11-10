#version 330 core
in vec3 startColor;
in vec3 endColor;
out vec4 FragColor;

uniform float t;

void main() {
  FragColor = vec4(t * startColor + (1 - t) * endColor, 1.0f);
}
