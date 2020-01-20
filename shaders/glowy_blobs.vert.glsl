#version 150

// Transforms and projects vertices, calculates parameters for lighting,
// Passes through blob parameters to fragment shader

// Attributes: Position, normal, texture coordinates
in vec3 vertexIn;
in vec3 normalIn;
in vec2 texcoordsIn;
in float blobglowIn;

// Same for the whole renderpass: Projection and Modelview(/normalview) matrices
uniform mat4 projection;
uniform mat4 modelview;
uniform mat4 normalview;

// To fragment shader
out vec3 objectPos;
out vec3 worldPos;
out vec3 normal;
out vec2 texcoords;
out float blobglow;

void main() {
    // Pass object-space pos / texcoords / glow
    objectPos = vertexIn;
    texcoords = texcoordsIn;
    blobglow = blobglowIn;
    
    // Transform the vertex according to modelview
    vec4 viewVertex;
    viewVertex = modelview * vec4(vertexIn, 1.0);
    worldPos = viewVertex.xyz;

    // Transform normal to world space
    normal = (normalview * vec4(normalIn, 0.0)).xyz;

    // Project and send to the fragment shader
    gl_Position = projection * viewVertex;
}
