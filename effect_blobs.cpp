#include "main.h"

#define BLOB_EXTENT 20
#define BLOB_COUNT 3

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

static GLuint glowyTexture;

typedef struct blobInfo {
    const sync_track* t;
    glm::vec3 pos;
} blobInfo;
static blobInfo blobs[3];

static const sync_track* radius;

typedef struct vertexInfo {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texcoord;
} vertexInfo;

static vertexInfo* blobVertices;

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

void cubeAt(vertexInfo* buffer, glm::vec3 at, glm::vec3 size) {
    for(int i = 0; i < 36; i++) {
        vertexInfo cubeVert = cube[i];
        cubeVert.pos = cubeVert.pos * size + at;
        buffer[i] = cubeVert;
    }
}

void effectBlobsInitialize() {
    // Basic OpenGL state
    glClearColor(0.2f, 0.1f, 0.3f, 1.0f);
    
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
    blobVertices = (vertexInfo*)malloc(sizeof(vertexInfo) * 36 * BLOB_EXTENT * BLOB_EXTENT * BLOB_EXTENT);
    quadBO = makeBO(GL_ARRAY_BUFFER, blobVertices, sizeof(vertexInfo) * 36 * BLOB_EXTENT * BLOB_EXTENT * BLOB_EXTENT, GL_DYNAMIC_DRAW);

    // Sync
    camRotX = sync_get_track(rocket, "blobs:rot.x");
    camRotY = sync_get_track(rocket, "blobs:rot.y");
    camRotZ = sync_get_track(rocket, "blobs:rot.z");
    blobs[0].t = sync_get_track(rocket, "blobs:blob1.t");
    blobs[1].t = sync_get_track(rocket, "blobs:blob2.t");
    blobs[2].t = sync_get_track(rocket, "blobs:blob3.t");
    radius = sync_get_track(rocket, "blobs:radius");
    
    // Textures
    glowyTexture = loadTexture("texture/glowy.tga");
}

glm::vec3 blobPos(float t, int i) {
    if(i == 0) {
        return glm::vec3(sin(t) + 2.0f * sin(2.0f * t), cos(t) + 2.0f * cos(2.0f * t), -sin(3.0f * t)) * 2.85;
    }
    else if (i == 1) {
        return glm::vec3(-sin(3.0f * t), sin(t) + 2.0f * sin(2.0f * t), cos(t) + 2.0f * cos(2.0f * t)) * 2.85;
    }
    return glm::vec3(cos(t) + 2.0f * cos(2.0f * t), -sin(3.0f * t), sin(t) + 2.0f * sin(2.0f * t)) * 2.85;
}

void effectBlobsRender() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    double bassRow = bassGetRow(stream);

    // Bind shader and set up uniforms
    glUseProgram(shaderProgram);
    
    glm::mat4 projection = glm::perspective(90.0f, (float)screenWidth / (float)screenHeight, 0.1f, 50.0f);
    glm::mat4 modelview = glm::lookAt(glm::vec3(0.0f, 0.0f, -20.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
    modelview *= (glm::mat4)glm::quat(glm::vec3(sync_get_val(camRotX, bassRow), sync_get_val(camRotY, bassRow), sync_get_val(camRotZ, bassRow))); 
    glm::mat4 normalview = glm::transpose(glm::inverse(modelview));
    
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(modelviewLoc, 1, GL_FALSE, glm::value_ptr(modelview));
    glUniformMatrix4fv(normalviewLoc, 1, GL_FALSE, glm::value_ptr(normalview));

    glUniform1i(glGetUniformLocation(shaderProgram, "textureIn"), 0);
    
    // Update blob pos
    for(int i = 0; i < BLOB_COUNT; i++) {
       blobs[i].pos = blobPos((float)sync_get_val(blobs[i].t, bassRow), i);
    }
    float blobRadius = (float)sync_get_val(radius, bassRow);

    // Update geometry
    srand(666);
    for(int x = 0; x < BLOB_EXTENT; x++) {
        for(int y = 0; y < BLOB_EXTENT; y++) {
            for(int z = 0; z < BLOB_EXTENT; z++) {
                glm::vec3 cubeCenter = glm::vec3(
                    x - BLOB_EXTENT / 2 + randFloatUnit() * 0.3, 
                    y - BLOB_EXTENT / 2 + randFloatUnit() * 0.3, 
                    z - BLOB_EXTENT / 2 + randFloatUnit() * 0.3
                );

                float blobValue = 0.0;
                for(int i = 0; i < BLOB_COUNT; i++) {
                    blobValue += blobRadius / (float)glm::length(cubeCenter + blobs[i].pos);
                }
                blobValue = pow(fmin(blobValue, 1.0f), 10.0f);
                blobValue = blobValue < 0.2f ? 0.0f : blobValue;

                cubeAt(
                    &blobVertices[(x + y * BLOB_EXTENT + z * BLOB_EXTENT * BLOB_EXTENT) * 36], 
                    cubeCenter, 
                    glm::vec3((randFloat() * 0.3f + 0.5f) * blobValue)
                );
            }
        }
    }

    // Set up texture
    glActiveTexture(GL_TEXTURE0);  
    glBindTexture(GL_TEXTURE_2D, glowyTexture);
    
    // Bind buffers and draw
    glBindBuffer(GL_ARRAY_BUFFER, quadBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexInfo) * 36 * BLOB_EXTENT * BLOB_EXTENT * BLOB_EXTENT, blobVertices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(vertexInLoc);
    glVertexAttribPointer(vertexInLoc, 3, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 0));

    glEnableVertexAttribArray(normalInLoc);
    glVertexAttribPointer(normalInLoc, 3, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 3));

    glEnableVertexAttribArray(texcoordsInLoc);
    glVertexAttribPointer(texcoordsInLoc, 2, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 6));

    glDrawArrays(GL_TRIANGLES, 0, 36 * BLOB_EXTENT * BLOB_EXTENT * BLOB_EXTENT);
}

void effectBlobsTerminate() {
    glDeleteTextures(1, &glowyTexture);
    free(blobVertices);
    glDeleteBuffers(1, &quadBO);
    glDeleteProgram(shaderProgram);
}
