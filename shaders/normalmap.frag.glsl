#version 150 core

// Fragment shader that uses a normal map

// Input
uniform sampler2D textureCol;
uniform sampler2D textureNorm;
uniform mat4 modelview;
uniform mat4 normalview;

// From vertex shader
in vec3 objectPos;
in vec3 worldPos;

// Output
out vec4 outColor;

void main() {
    vec2 texcoords = objectPos.xz / 10.0f;

    // Sample texture for base color
    vec4 colIn = texture(textureCol, texcoords);
    vec3 normal = texture(textureNorm, texcoords).xzy;
    normal = normalize(normal - vec3(0.5f));

    // Point light
    vec3 lightPos = vec3(0.0f, 1.0f, 0.0f);
    float lightDist = length(lightPos);
    vec3 lightDir = lightPos / lightDist;
    float attenuate = (1.0f + 1.0f * pow(lightDist, 2.0f));

    float lambert = max(0.0f, dot(lightDir, normal)) * 0.2f;
    lambert = lambert / attenuate;

    vec3 refLightDir = reflect(-lightDir, normal);
    vec4 eye = inverse(modelview) * vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec3 eyeDir = normalize(eye.xyz - objectPos);

    float phong = pow(max(dot(refLightDir, eyeDir), 0.0f), 40.0f);

    outColor = vec4(colIn.xyz * lambert, 1.0) + vec4(phong / attenuate);
}
