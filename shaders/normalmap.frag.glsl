#version 150 core

// Fragment shader that uses a normal map

// Input
uniform sampler2D textureCol;
uniform sampler2D textureNorm;

// From vertex shader
in vec3 objectPos;
in vec3 worldPos;

// Output
out vec4 outColor;

void main() {
    vec2 texcoords = objectPos.xz / 10.0f;

    // Sample texture for base color
    vec4 colIn = texture(textureCol, texcoords);
    vec3 normal = normalize((vec4(texture(textureNorm, texcoords).xyz, 0.0f)).xyz);

    // Sun is at infinity
    vec3 lightDir = vec3(1.0f, 1.0f, 1.0f);
    float lambert = max(0.0f, dot(normalize(lightDir), normal));
    float light = min(lambert + 0.1f, 1.0f);

    outColor = vec4(colIn.xyz * light * 0.2f, 1.0);
}
