#version 330 core

struct Material {
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};

struct Light {
  vec3 pos;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;
out vec4 FragColor;

uniform mat4 normalMatrix;
uniform Material material;
uniform Light light;

vec3 calculateAmbient() {
    return vec3(texture(material.diffuse, texCoord)) * light.ambient;
}

vec3 calculateDiffuse(vec3 normalizedNormal, vec3 lightDir) {
    float diffuseFactor = max(dot(normalizedNormal, lightDir), 0.0);
    return (diffuseFactor * vec3(texture(material.diffuse, texCoord))) * light.diffuse;
}

vec3 calculateSpecular(vec3 normalizedNormal, vec3 lightDir, vec3 viewDir) {
    vec3 reflectDir = reflect(-lightDir, normalizedNormal);
    float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    return (specularFactor * vec3(texture(material.specular, texCoord))) * light.specular;
}

void main() {
    // Transform normal to world space
    vec3 normalWorld = normalize(vec3(normalMatrix * vec4(normal, 0.0)));
    
    // Calculate lighting vectors
    vec3 lightDir = normalize(light.pos - fragPos);
    vec3 viewDir = normalize(-fragPos);
    
    // Calculate lighting components
    vec3 ambient = calculateAmbient();
    vec3 diffuse = calculateDiffuse(normalWorld, lightDir);
    vec3 specular = calculateSpecular(normalWorld, lightDir, viewDir);
    
    // Combine all components
    vec3 finalColor = ambient + diffuse + specular;
    FragColor = vec4(finalColor, 1.0);
}
