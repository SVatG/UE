#version 150 core

// Glowing-particle fragment shader

// Input
uniform vec4 colorGlow;

// From vertex shader
in vec3 worldPos;

// Output
out vec4 outColor;
out vec4 outDepth;

void main() {
    // Distance to point center
    float pointdist = length(gl_PointCoord - vec2(0.5f)) * 2.0f;

    // Output glow color, soften to edge
    outColor = colorGlow;
    outColor.a = pow((1.0f - min(1.0f, pointdist)), 5.0f);

    // Calculate and output depth
    float focusdepth = 5.5f;
    float focallength = 3.0f;
    float lensdiameter = 0.1f;
    float depth = -worldPos.z;
    float coc = lensdiameter * (abs(depth - focusdepth) / depth) * (focallength / (focusdepth - focallength));
    coc = abs(coc);
    coc = coc < 0.01f ? 0.01f : coc;

    outDepth.a = coc;
}
