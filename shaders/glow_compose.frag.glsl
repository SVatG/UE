#version 150

in vec2 texcoord;
uniform sampler2D baseTex;
uniform sampler2D glowTex;
out vec4 outColor;

float hash( float n ) {
    return fract(sin(n)*687.3123);
}

float noise( in vec2 x ) {
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*157.0;
    return mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
        mix( hash(n+157.0), hash(n+158.0),f.x),f.y);
}

void main() {
    vec2 coords = (texcoord - vec2(0.5)) * 2.0f;

    float vignette = max(0.0f, 0.9f - pow(length(coords / 2.0f), 0.8f));
    vec4 glowCol = texture2D(glowTex, texcoord);
    outColor = (texture2D(baseTex, texcoord) + glowCol) * vignette;
}
