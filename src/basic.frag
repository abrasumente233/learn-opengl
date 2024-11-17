#version 330 core

in vec3 normal;
in vec3 fragPos;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform mat4 normalMatrix;

void main() {
  float ambientStrength = 0.1f;
  vec3 ambient = ambientStrength * lightColor;

  vec3 norm = normalize(vec3(normalMatrix * vec4(normal, 0.0f)));
  vec3 lightDirection = normalize(lightPos - fragPos);
  float diffuseStrength = max(dot(norm, lightDirection), 0.0f);
  vec3 diffuse = diffuseStrength * lightColor;

  FragColor = vec4((ambient + diffuse) * objectColor, 1.0f);
}
