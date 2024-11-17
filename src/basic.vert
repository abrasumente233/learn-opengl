#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 normal;
out vec3 fragPos;
out float phong;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform mat4 normalMatrix;

void main() {
  normal = aNormal;
  vec4 viewPos = view * model * vec4(aPos, 1.0);
  fragPos = viewPos.xyz;
  gl_Position = projection * viewPos;

  // Phong shading
  // ambient
  float ambient = 0.1f;

  // diffuse
  vec3 norm = normalize(vec3(normalMatrix * vec4(normal, 0.0f)));
  vec3 lightDirection = normalize(lightPos - fragPos);
  float diffuse = max(dot(norm, lightDirection), 0.0f);

  // specular
  float specularStrength = 0.5f;
  vec3 viewDirection = normalize(-fragPos);
  vec3 reflectDirection = reflect(-lightDirection, norm);
  float spec = pow(max(dot(viewDirection, reflectDirection), 0.0f), 256);
  float specular = specularStrength * spec;

  phong = ambient + diffuse + specular;
}
