#version 150 core

// Very simple one color fragment shader

// Input
uniform vec4 color;
uniform vec4 colorGlow;

// From vertex shader
in vec3 objectPos;
in vec3 worldPos;
in vec3 normal;

// Output
out vec4 outColor;

void main() {
    // Sun is at infinity
    vec3 lightDir = vec3(1.0f, 1.0f, 1.0f);
    float lambert = max(0.0f, dot(normalize(lightDir), normalize(normal)));
    float light = min(lambert + 0.1f, 1.0f);

    // Output given color
    outColor = color * light + colorGlow;
}
