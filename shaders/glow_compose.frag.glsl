#version 150

// Input
in vec2 texcoord;
uniform sampler2D baseTex;
uniform sampler2D glowTex;
uniform sampler2D floorTex;
uniform mat4 normalviewCamera;
uniform float bassRow;

// Output
out vec4 outColor;

float hash(float n) {
    return fract(sin(n) * 687.3123f);
}

float noise(in vec2 x ) {
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f * f * (3.0f - 2.0f * f);
    float n = p.x + p.y * 157.0f;
    return mix(mix(hash(n + 0.0f), hash(n + 1.0f), f.x),
        mix(hash(n + 157.0f), hash(n + 158.0f), f.x), f.y);
}

void main() {
    vec2 coords = (texcoord - vec2(0.5f)) * 2.0f;
    float floor = texture2D(floorTex, texcoord).a > 0.7f ? 1.0f : 0.0f;
    vec2 noiseTexcoord = texcoord + noise(coords * 1000000.0f + vec2(bassRow)) * 0.005f * floor;

    float vignette = max(0.0f, 0.9f - pow(length(coords / 2.0f), 0.8f));
    vec4 glowCol = texture2D(glowTex, noiseTexcoord);
    outColor = (texture2D(baseTex, noiseTexcoord) + glowCol) * vignette;

    vec2 rainDir = normalize((normalviewCamera * vec4(0.0f, 1.0f, 0.0f, 0.0f)).xy);
    float rainAngle = rainDir.x;
    mat2 rainRot = mat2(
            cos(rainAngle), -sin(rainAngle),
            sin(rainAngle),  cos(rainAngle)
    );
    vec2 rainCoord = coords * rainRot;
    rainCoord = rainCoord * vec2(300.0f, 5.0f) + vec2(0.0f, bassRow * 10.0f);
    float rainfall = pow(noise(rainCoord), 20.0f) * 0.1f;
    outColor += vec4(rainfall) + glowCol * pow(rainfall, 0.3f);
}
