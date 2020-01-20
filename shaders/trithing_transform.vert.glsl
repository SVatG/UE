#version 150

// Triangular object shader

// Attributes: Position, normal
in vec3 vertexIn;
in vec3 normalIn;

// Same for the whole renderpass: Projection and Modelview(/normalview) matrices
uniform mat4 projection;
uniform mat4 modelview;
uniform mat4 normalview;

// To fragment shader
out vec3 objectPos;
out vec3 worldPos;
out vec3 normal;

void main() {
    // Pass object-space pos / texcoords
    objectPos = vertexIn;

    // Transform the vertex according to modelview
    vec4 viewVertex;
    viewVertex = modelview * vec4(vertexIn, 1.0);
    worldPos = viewVertex.xyz;

    // Transform normal to world space
    normal = normalize((normalview * vec4(normalIn, 0.0)).xyz);

    // Project and send to the fragment shader
    gl_Position = projection * viewVertex;
}
