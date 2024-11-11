#version 330 core
in vec3 startColor;
in vec3 endColor;
in vec2 texCoord;
out vec4 FragColor;

uniform float t;
uniform sampler2D texture0;
uniform sampler2D texture1;

void main() {
  // FragColor = vec4(t * startColor + (1 - t) * endColor, 1.0f);
  vec4 ourColor = vec4(t * startColor + (1 - t) * endColor, 1.0f);
  vec4 tex0 = texture(texture0, texCoord);
  vec4 tex1 = texture(texture1, texCoord);
  // vec4 tex1 = texture(texture1, vec2(1.0 - texCoord.x, texCoord.y));
  FragColor = mix(tex0, tex1, 0.2);
}
