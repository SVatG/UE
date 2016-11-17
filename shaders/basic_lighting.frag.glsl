#version 150 core

// Simple fragment shader.
// Does (TODO: texturing and) diffuse shading with a fixed-direction sun.

// From vertex shader
in vec3 objectPos;
in vec3 worldPos;
in vec3 normal;
in vec2 texcoords;

// Output
out vec4 outColor;

void main() {

    // Sun is at infinity
    vec3 lightDir = vec3(1.0f, 1.0f, 1.0f);
    float lambert = max(0.0f, dot(normalize(lightDir), normal));
    float light = lambert + 0.3f;

    outColor = vec4(light);
}