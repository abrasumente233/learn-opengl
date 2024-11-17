#version 330 core

in vec3 normal;
in vec3 fragPos;
in float phong;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform mat4 normalMatrix;

void main() {
  // ambient
  float ambientStrength = 0.1f;
  vec3 ambient = ambientStrength * lightColor;

  // diffuse
  vec3 norm = normalize(vec3(normalMatrix * vec4(normal, 0.0f)));
  vec3 lightDirection = normalize(lightPos - fragPos);
  float diffuseStrength = max(dot(norm, lightDirection), 0.0f);
  vec3 diffuse = diffuseStrength * lightColor;

  // specular
  float specularStrength = 0.5f;
  vec3 viewDirection = normalize(-fragPos);
  vec3 reflectDirection = reflect(-lightDirection, norm);
  float spec = pow(max(dot(viewDirection, reflectDirection), 0.0f), 32);
  vec3 specular = specularStrength * spec * lightColor;

  // FragColor = vec4((ambient + diffuse + specular) * objectColor, 1.0f);
  FragColor = vec4(phong * lightColor * objectColor, 1.0f);
}
