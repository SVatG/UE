#version 150 core

// Very simple one color fragment shader

// Input
uniform vec4 color;
uniform vec4 colorGlow;
uniform mat4 cameraTransform;

// From vertex shader
in vec3 objectPos;
in vec3 worldPos;
in vec3 normal;

// Output
out vec4 outColor;

void main() {
    vec3 normalProper = normalize(normal);

    // Basic diffuse lighting
    vec3 lightPos = (cameraTransform * vec4(0.0f, 3.0, 0.0f, 1.0f)).xyz;
    vec3 lightVec = lightPos - worldPos;
    float lightDist = length(lightVec);
    vec3 lightDir = lightVec / lightDist;
    float attenuate = (1.0f + 0.05f * pow(lightDist, 2.0f));

    float lambert = max(0.0f, dot(lightDir, normalProper)) * 2.0f;
    lambert = lambert / attenuate;

    // Output given color
    outColor = color * lambert + colorGlow;

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
