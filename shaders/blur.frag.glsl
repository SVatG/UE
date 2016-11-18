#version 150

uniform vec2 resolution;
uniform vec2 direction;

in vec2 texcoord;
uniform sampler2D tex;
out vec4 outColor;

void main() {
    vec4 color = vec4(0.0);

    vec2 off1 = vec2(1.411764705882353) * direction;
    vec2 off2 = vec2(3.2941176470588234) * direction;
    vec2 off3 = vec2(5.176470588235294) * direction;
    color += texture2D(tex, texcoord) * 0.1964825501511404;
    color += texture2D(tex, texcoord + (off1 / (resolution * 0.5f))) * 0.2969069646728344;
    color += texture2D(tex, texcoord - (off1 / (resolution * 0.5f))) * 0.2969069646728344;
    color += texture2D(tex, texcoord + (off2 / (resolution * 0.5f))) * 0.09447039785044732;
    color += texture2D(tex, texcoord - (off2 / (resolution * 0.5f))) * 0.09447039785044732;
    color += texture2D(tex, texcoord + (off3 / (resolution * 0.5f))) * 0.010381362401148057;
    color += texture2D(tex, texcoord - (off3 / (resolution * 0.5f))) * 0.010381362401148057;

    outColor = color;
}
