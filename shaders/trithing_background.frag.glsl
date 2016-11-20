#version 150

// Input
in vec2 texcoord;
uniform float bassRow;
uniform float lightUp;

// Output
out vec4 outColor;

void main() {
    float lineWidth = 7.0f;
    float lineDuty = 8.0f;
    float maxSegments = 32.0f;

    vec4 baseCol = vec4(0.1f, 0.0f, 0.1f, 1.0f);
    vec4 lineCol = vec4(0.8f, 0.7f, 0.8f, 1.0f);
    vec4 lightUpCol = vec4(0.95f, 0.3f, 0.85f, 1.0f);

    float currentCol = floor(texcoord.x * 1280.0f) + bassRow * 10.0f;
    float currentLine = floor(texcoord.y * 720.0f); // TODO uniform this
    float offsetDir = -(texcoord.x - 0.5f) * 2.0f;

    float offset = 0.0f;
    if(currentLine < 200) {
        offset = (currentLine - 200f);
    }

    if(currentLine > 720 - 200) {
        offset = -(currentLine - (720 - 200));
    }

    currentCol -= offset * offsetDir;

    float currentColProper = mod(floor(currentCol / lineWidth), lineDuty);
    float currentSegment = mod(floor(currentCol / (lineWidth * lineDuty)), maxSegments);
    outColor = baseCol;
    if(currentColProper == 0.0f) {
        outColor = lineCol;
    }
    else{
        if(currentSegment == floor(mod(lightUp, maxSegments))) {
            outColor = lightUpCol;
        }
    }
}
