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

    // Calculate and output depth
    float focusdepth = 5.5f;
    float focallength = 3.0f;
    float lensdiameter = 0.1f;
    float depth = -worldPos.z;
    float coc = lensdiameter * (abs(depth - focusdepth) / depth) * (focallength / (focusdepth - focallength));
    coc = abs(coc);
    coc = coc < 0.01f ? 0.01f : coc;

    outColor.a = coc;
}
