#version 150 core

// Simple fragment shader.
// Does texturing and diffuse shading with a fixed-direction sun.

// Input
uniform sampler2D textureCol;
uniform mat4 modelview;
uniform mat4 normalview;
uniform mat4 cameraTransform;

// From vertex shader
in vec3 objectPos;
in vec3 worldPos;
in vec3 normal;
in vec2 texcoords;

// Output
out vec4 outColor;

void main() {
    // Sample texture for base color
    vec4 colIn = texture(textureCol, texcoords);
    vec3 normalProper = normalize(normal);

    // Point light
    vec3 lightPos = (cameraTransform * vec4(0.0f, 10.0f, 0.0f, 1.0f)).xyz;
    vec3 lightVec = lightPos - worldPos;
    float lightDist = length(lightVec);
    vec3 lightDir = lightVec / lightDist;
    float attenuate = (1.0f + 0.01f * pow(lightDist, 2.0f));

    float lambert = max(0.0f, dot(lightDir, normalProper));
    lambert = lambert / attenuate;

    vec3 refLightDir = reflect(-lightDir, normalProper);
    vec3 eyeDir = normalize(-worldPos);

    float phong = pow(max(dot(refLightDir, eyeDir), 0.0f), 40.0f);
    phong = phong / attenuate;

    outColor = vec4(colIn.xyz * lambert, 1.0) + vec4(phong);

    // Blue glows
    if(colIn.b > 0.5f) {
        outColor += colIn * 2.0f;
    }

    float alpha = 1.0f - pow(max(0.0f, abs(objectPos.y / 12.0f)) * 0.3f, 2.0f);
    outColor.a = alpha * 0.1f;
}
