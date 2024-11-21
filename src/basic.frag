#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirectionalLight {
    vec3 dir;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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
uniform DirectionalLight directionalLight;

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

vec3 calculateSpotlight(Spotlight spotlight, vec3 fragDir, vec3 viewDir, vec3 normalView) {
    float theta = dot(normalize(-spotlight.dir), fragDir);
    float eps = spotlight.cutoff - spotlight.outerCutoff;
    float intensity = clamp((theta - spotlight.outerCutoff) / eps, 0.0f, 1.0f);

    vec3 ambient = calculateAmbient(spotlight.ambient);
    vec3 diffuse = calculateDiffuse(spotlight.diffuse, normalView, fragDir);
    vec3 specular = calculateSpecular(spotlight.specular, normalView, fragDir, viewDir);

    return ambient + (diffuse + specular) * intensity;
}

vec3 calculateDirectionalLight(DirectionalLight light, vec3 viewDir, vec3 normalView) {
    vec3 lightDirWorld = normalize(-light.dir);
    vec3 lightDirView = normalize(vec3(normalMatrix * vec4(lightDirWorld, 0.0)));

    vec3 ambient = calculateAmbient(light.ambient);
    vec3 diffuse = calculateDiffuse(light.diffuse, normalView, lightDirView);
    vec3 specular = calculateSpecular(light.specular, normalView, lightDirView, viewDir);

    return ambient + diffuse + specular;
}

void main() {
    // Transform normal to view space
    vec3 normalView = normalize(vec3(normalMatrix * vec4(normal, 0.0)));

    // Calculate lighting vectors
    vec3 fragDir = normalize(spotlight.pos - fragPos);
    vec3 viewDir = normalize(-fragPos);

    vec3 finalColor = vec3(0.0f);
    finalColor += calculateSpotlight(spotlight, fragDir, viewDir, normalView);
    finalColor += calculateDirectionalLight(directionalLight, viewDir, normalView);

    FragColor = vec4(finalColor, 1.0f);
}
