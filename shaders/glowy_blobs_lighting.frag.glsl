#version 150 core

// Simple fragment shader.
// Does texturing and diffuse shading with a fixed-direction sun.

// Input
uniform sampler2D textureCol;

// From vertex shader
in vec3 objectPos;
in vec3 worldPos;
in vec3 normal;
in vec2 texcoords;
in float blobglow;

// Output
out vec4 outColor;

void main() {
    // Sample texture for base color
    vec4 colIn = texture(textureCol, texcoords);
    
    // Sun is at infinity
    vec3 lightDir = vec3(1.0f, 1.0f, 1.0f);
    float lambert = max(0.0f, dot(normalize(lightDir), normal));
    float light = min(lambert + 0.2f, 1.0f);

    outColor = vec4(colIn.xyz * light * (1.0f + blobglow * 0.5f), worldPos.z);
}