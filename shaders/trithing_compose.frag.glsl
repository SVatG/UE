#version 150

// Input
in vec2 texcoord;
uniform sampler2D baseTex;

// Output
out vec4 outColor;

void main() {
    float curLine = floor(mod(texcoord.y * 720.0f, 4.0f) < 3.0f ? 0.0f : 1.0f);
    outColor = texture(baseTex, texcoord) * (curLine / 2.0f + 0.5f);
}
