#version 330 core

in vec2 texCoord;
out vec4 FragColor;

uniform float t;
uniform float mixValue;
uniform sampler2D texture0;
uniform sampler2D texture1;

void main() {
  vec4 tex0 = texture(texture0, texCoord);
  vec4 tex1 = texture(texture1, texCoord);
  FragColor = mix(tex0, tex1, mixValue);
}
