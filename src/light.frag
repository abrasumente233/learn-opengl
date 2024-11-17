#version 330 core

// in vec3 normal;
// in vec3 fragPos;
in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D lampTexture;

void main() {
  FragColor = texture(lampTexture, texCoord);
}
