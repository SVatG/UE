#version 150

// Input
in vec2 texcoord;
uniform sampler2D baseTex;
uniform sampler2D depthTex;

// Output
out vec4 outColor;

// blur with hexagonalish sampling pattern
// weighs samples according to coc size (so that "in focus" samples count for less)
// and according to tap nb (weighs outer samples higher)
vec3 hexablur(sampler2D tex, sampler2D texd, vec2 uv) {
    vec2 scale = vec2(1.0f) / vec2(1280.0f, 720.0f);
    vec3 col = vec3(0.0f);
    float asum = 0.0f;
    float coc = texture2D(texd, uv).a;
    for(float t = 0.0f; t < 8.0f * 2.0f * 3.14f; t += 3.14f / 32.0f) {
        float r = cos(3.14f / 6.0f) / cos(mod(t, 2.0f * 3.14f / 6.0f) - 3.14f / 6.0f);

        // Tap filter once for coc
        vec2 offset = vec2(sin(t), cos(t)) * r * t * scale * coc;
        vec4 samp = texture2D(texd, uv + offset * 1.0f);

        // Tap filter with coc from texture
        offset = vec2(sin(t), cos(t)) * r * t * scale * samp.a;
        samp = texture2D(tex, uv + offset * 1.0f);
        vec4 sampd = texture2D(texd, uv + offset * 1.0f);

        // weigh and save
        col += samp.rgb * sampd.a * t;
        asum += sampd.a * t;

    }
    col = col / asum;
    return(col);
}

void main() {
    float curLine = floor(mod(texcoord.y * 720.0f, 4.0f) < 3.0f ? 0.0f : 1.0f);
    outColor = vec4(hexablur(baseTex, depthTex, texcoord) * (curLine / 2.0f + 0.5f), 1.0f);
    
    // Tonemap, gamma
    outColor = outColor / (outColor + vec4(1.0));
}
