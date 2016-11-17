#include "main.h"

static GLuint shaderProgram;
static GLuint quadBO;

static GLuint projectionLoc;
static GLuint modelviewLoc;
static GLuint normalviewLoc;

static GLuint vertexInLoc;
static GLuint normalInLoc;
static GLuint texcoordsInLoc;

static const sync_track* camRotX;
static const sync_track* camRotY;
static const sync_track* camRotZ;

typedef struct vertexInfo {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texcoord;
} vertexInfo;

// Too-lazy-for-element-buffer quads
vertexInfo cube[] = {
    // Front
    {{-1, -1,  1}, {0, 0,  1}, {0, 0}},
    {{ 1, -1,  1}, {0, 0,  1}, {1, 0}},
    {{-1,  1,  1}, {0, 0,  1}, {0, 1}},
    {{ 1, -1,  1}, {0, 0,  1}, {1, 0}},
    {{ 1,  1,  1}, {0, 0,  1}, {1, 1}},
    {{-1,  1,  1}, {0, 0,  1}, {0, 1}},

    // Back
    {{-1, -1, -1}, {0, 0, -1}, {0, 0}},
    {{-1,  1, -1}, {0, 0, -1}, {0, 1}},
    {{ 1, -1, -1}, {0, 0, -1}, {1, 0}},
    {{ 1, -1, -1}, {0, 0, -1}, {1, 0}},    
    {{-1,  1, -1}, {0, 0, -1}, {0, 1}},
    {{ 1,  1, -1}, {0, 0, -1}, {1, 1}},

    // Left
    {{-1, -1, -1}, {-1, 0, 0}, {0, 0}},
    {{-1, -1,  1}, {-1, 0, 0}, {0, 1}},
    {{-1,  1, -1}, {-1, 0, 0}, {1, 0}},
    {{-1,  1, -1}, {-1, 0, 0}, {1, 0}},
    {{-1, -1,  1}, {-1, 0, 0}, {0, 1}},
    {{-1,  1,  1}, {-1, 0, 0}, {1, 1}},

    // Right
    {{ 1, -1, -1}, { 1, 0, 0}, {0, 0}},
    {{ 1,  1, -1}, { 1, 0, 0}, {1, 0}},
    {{ 1, -1,  1}, { 1, 0, 0}, {0, 1}},
    {{ 1,  1, -1}, { 1, 0, 0}, {1, 0}},
    {{ 1,  1,  1}, { 1, 0, 0}, {1, 1}},
    {{ 1, -1,  1}, { 1, 0, 0}, {0, 1}},

    // Top
    {{-1, -1, -1}, { 0, -1, 0}, {0, 0}},
    {{ 1, -1, -1}, { 0, -1, 0}, {1, 0}},
    {{-1, -1,  1}, { 0, -1, 0}, {0, 1}},
    {{ 1, -1, -1}, { 0, -1, 0}, {1, 0}},
    {{ 1, -1,  1}, { 0, -1, 0}, {1, 1}},
    {{-1, -1,  1}, { 0, -1, 0}, {0, 1}},

    // Bottom
    {{-1, 1, -1}, { 0, 1, 0}, {0, 0}},
    {{-1, 1,  1}, { 0, 1, 0}, {0, 1}},
    {{ 1, 1, -1}, { 0, 1, 0}, {1, 0}},
    {{ 1, 1, -1}, { 0, 1, 0}, {1, 0}},
    {{-1, 1,  1}, { 0, 1, 0}, {0, 1}},
    {{ 1, 1,  1}, { 0, 1, 0}, {1, 1}},
};

void effectBlobsInitialize() {
    // Basic OpenGL state
    glClearColor(1.0, 0.0, 0.0, 1.0);
    
    // A shader
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, "shaders/basic.vert.glsl");
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "shaders/basic_lighting.frag.glsl");
    shaderProgram = makeShaderProgram(fragmentShader, vertexShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Uniforms
    projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    modelviewLoc = glGetUniformLocation(shaderProgram, "modelview");
    normalviewLoc = glGetUniformLocation(shaderProgram, "normalview");

    // Attributes
    vertexInLoc = glGetAttribLocation(shaderProgram, "vertexIn");
    normalInLoc = glGetAttribLocation(shaderProgram, "normalIn");
    texcoordsInLoc = glGetAttribLocation(shaderProgram, "texcoordsIn");

    // Geometry
    quadBO = makeBO(GL_ARRAY_BUFFER, cube, sizeof(vertexInfo) * 36, GL_STATIC_DRAW);

    // Sync
    camRotX = sync_get_track(rocket, "blobs:rot.x");
    camRotY = sync_get_track(rocket, "blobs:rot.y");
    camRotZ = sync_get_track(rocket, "blobs:rot.z");
}

void effectBlobsRender() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Bind shader and set up uniforms
    glUseProgram(shaderProgram);
    
    glm::mat4 projection = glm::perspective(90.0f, (float)screenWidth / (float)screenHeight, 0.1f, 50.0f);

    double bassRow = bassGetRow(stream);
    glm::mat4 modelview = glm::lookAt(glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
    modelview *= (glm::mat4)glm::quat(glm::vec3(sync_get_val(camRotX, bassRow), sync_get_val(camRotY, bassRow), sync_get_val(camRotZ, bassRow))); 
    glm::mat4 normalview = glm::transpose(glm::inverse(modelview));
    
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(modelviewLoc, 1, GL_FALSE, glm::value_ptr(modelview));
    glUniformMatrix4fv(normalviewLoc, 1, GL_FALSE, glm::value_ptr(normalview));

    // Bind buffers and draw
    glBindBuffer(GL_ARRAY_BUFFER, quadBO);

    glEnableVertexAttribArray(vertexInLoc);
    glVertexAttribPointer(vertexInLoc, 3, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 0));

    glEnableVertexAttribArray(normalInLoc);
    glVertexAttribPointer(normalInLoc, 3, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 3));

    glEnableVertexAttribArray(texcoordsInLoc);
    glVertexAttribPointer(texcoordsInLoc, 2, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 6));

    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void effectBlobsTerminate() {
   glDeleteProgram(shaderProgram);
}
