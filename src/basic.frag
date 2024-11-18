#version 330 core

in vec3 normal;
in vec3 fragPos;
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform mat4 normalMatrix;

// Material properties could be uniforms instead of hardcoded
const float AMBIENT_STRENGTH = 0.1;
const float SPECULAR_STRENGTH = 0.5;
const float SHININESS = 32.0;

vec3 calculateAmbient() {
    return AMBIENT_STRENGTH * lightColor;
}

vec3 calculateDiffuse(vec3 normalizedNormal, vec3 lightDir) {
    float diffuseFactor = max(dot(normalizedNormal, lightDir), 0.0);
    return diffuseFactor * lightColor;
}

vec3 calculateSpecular(vec3 normalizedNormal, vec3 lightDir, vec3 viewDir) {
    vec3 reflectDir = reflect(-lightDir, normalizedNormal);
    float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), SHININESS);
    return SPECULAR_STRENGTH * specularFactor * lightColor;
}

void main() {
    // Transform normal to world space
    vec3 normalWorld = normalize(vec3(normalMatrix * vec4(normal, 0.0)));
    
    // Calculate lighting vectors
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 viewDir = normalize(-fragPos);
    
    // Calculate lighting components
    vec3 ambient = calculateAmbient();
    vec3 diffuse = calculateDiffuse(normalWorld, lightDir);
    vec3 specular = calculateSpecular(normalWorld, lightDir, viewDir);
    
    // Combine all components
    vec3 finalColor = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(finalColor, 1.0);
}
