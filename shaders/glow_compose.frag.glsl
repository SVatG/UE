#version 150

in vec2 texcoord;
uniform sampler2D baseTex;
uniform sampler2D glowTex;
out vec4 outColor;

void main() {
    float vignette = max(0.0f, 0.9f - pow(length(texcoord - vec2(0.5)), 0.8f));
    vec4 glowCol = texture2D(glowTex, texcoord);
    outColor = (texture2D(baseTex, texcoord) + glowCol) * vignette;
}
