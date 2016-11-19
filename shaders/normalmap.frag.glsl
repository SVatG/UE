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
out vec4 outFloor;

void main() {
    vec2 texcoords = objectPos.xz / 10.0f;

    // Sample texture for base color
    vec4 colIn = texture(textureCol, texcoords);
    vec3 normal = normalize(texture(textureNorm, texcoords).xzy - vec3(0.5f));
    normal = normalize((normalview * vec4(normal, 0.0f)).xyz);

    // Point light
    vec3 lightPos = (modelview * vec4(0.0f, 10.0f, 0.0f, 1.0f)).xyz;
    vec3 lightVec = lightPos - worldPos;
    float lightDist = length(lightVec);
    vec3 lightDir = lightVec / lightDist;
    float attenuate = (1.0f + 0.01f * pow(lightDist, 2.0f));

    float lambert = max(0.0f, dot(lightDir, normal));
    lambert = lambert / attenuate;

    vec3 refLightDir = reflect(-lightDir, normal);
    vec3 eyeDir = normalize(-worldPos);

    float phong = pow(max(dot(refLightDir, eyeDir), 0.0f), 40.0f);
    phong = phong / attenuate;

    outColor = vec4(colIn.xyz * lambert, 1.0) + vec4(phong);
    outFloor = vec4(1.0f);
}
