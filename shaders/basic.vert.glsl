#version 150

// Simple vertex shader.
// Transforms and projects vertices and calculates parameters for lighting.

// Attributes: Position, normal, texture coordinates
in vec3 vertexIn;
in vec3 normalIn;
in vec2 texcoordsIn;

// Same for the whole renderpass: Projection and Modelview(/normalview) matrices
uniform mat4 projection;
uniform mat4 modelview;
uniform mat4 normalview;

// To fragment shader
out vec3 objectPos;
out vec3 worldPos;
out vec3 normal;
out vec2 texcoords;

void main() {
    // Pass object-space pos / texcoords
    objectPos = vertexIn;
    texcoords = texcoordsIn;

    // Transform the vertex according to modelview
    vec4 viewVertex;
    viewVertex = modelview * vec4(vertexIn, 1.0);
    worldPos = viewVertex.xyz;

    // Transform normal to world space
    normal = (normalview * vec4(normalIn, 0.0)).xyz;

    // Project and send to the fragment shader
    gl_Position = projection * viewVertex;
}
