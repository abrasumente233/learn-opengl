#version 330 core

struct Material {
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};

struct Spotlight {
  vec3 pos;
  vec3 dir;
  float cutoff;
  float outerCutoff;

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
uniform Spotlight spotlight;

vec3 calculateAmbient(vec3 light_ambient) {
    return vec3(texture(material.diffuse, texCoord)) * light_ambient;
}

vec3 calculateDiffuse(vec3 light_diffuse, vec3 normalizedNormal, vec3 fragDir) {
    float diffuseFactor = max(dot(normalizedNormal, fragDir), 0.0);
    return (diffuseFactor * vec3(texture(material.diffuse, texCoord))) * light_diffuse;
}

vec3 calculateSpecular(vec3 light_specular, vec3 normalizedNormal, vec3 fragDir, vec3 viewDir) {
    vec3 reflectDir = reflect(-fragDir, normalizedNormal);
    float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    return (specularFactor * vec3(texture(material.specular, texCoord))) * light_specular;
}

vec3 calculateSpotlight(Spotlight spotlight, vec3 fragDir, vec3 viewDir, vec3 normalWorld) {
    float theta = dot(normalize(-spotlight.dir), fragDir);
    float eps = spotlight.cutoff - spotlight.outerCutoff;
    float intensity = clamp((theta - spotlight.outerCutoff) / eps, 0.0f, 1.0f);

    vec3 ambient = calculateAmbient(spotlight.ambient);
    vec3 diffuse = calculateDiffuse(spotlight.diffuse, normalWorld, fragDir);
    vec3 specular = calculateSpecular(spotlight.specular, normalWorld, fragDir, viewDir);

    return ambient + (diffuse + specular) * intensity;
}

void main() {
    // Transform normal to world space
    vec3 normalWorld = normalize(vec3(normalMatrix * vec4(normal, 0.0)));
    
    // Calculate lighting vectors
    vec3 fragDir = normalize(spotlight.pos - fragPos);
    vec3 viewDir = normalize(-fragPos);
    
    vec3 finalColor = calculateSpotlight(spotlight, fragDir, viewDir, normalWorld);

    FragColor = vec4(finalColor, 1.0f);
}
