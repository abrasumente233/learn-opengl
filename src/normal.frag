#version 330 core

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;
out vec4 FragColor;

uniform mat3 normalMatrix;

void main() {
    // Transform normal to view space
    vec3 normalView = normalize(normalMatrix * normal);

    // Map normal vectors from [-1,1] to [0,1] for visualization
    // vec3 color = normalize(normalView) * 0.5 + 0.5;
    vec3 color = normalize(normal) * 0.5 + 0.5;

    FragColor = vec4(color, 1.0);
}

