#include "main.h"
#include "objloader.h"

#define BLOB_EXTENT 20
#define BLOB_COUNT 3

static GLuint shaderProgram;
static GLuint floorShaderProgram;
static GLuint houseShaderProgram;
static GLuint blurShaderProgram;
static GLuint composeShaderProgram;
static GLuint cubesBO;
static GLuint floorBO;
static GLuint houseBO;

static size_t houseVertCount;

static GLuint projectionLoc;
static GLuint modelviewLoc;
static GLuint normalviewLoc;

static GLuint vertexInLoc;
static GLuint normalInLoc;
static GLuint texcoordsInLoc;
static GLuint blobglowInLoc;

static const sync_track* camRotX;
static const sync_track* camRotY;
static const sync_track* camRotZ;

static GLuint glowyTexture;
static GLuint brickTexture;
static GLuint brickNormalTexture;
static GLuint houseTexture;

static GLuint postprocTextureInitial;
static GLuint postprocFBOInitial;

static GLuint postprocTextureA;
static GLuint postprocFBOA;

static GLuint postprocTextureB;
static GLuint postprocFBOB;

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
    float glow;
} vertexInfo;

static vertexInfo* blobVertices;

// Too-lazy-for-element-buffer quads
vertexInfo cubeVertices[] = {
    // Front
    {{-1, -1,  1}, {0, 0,  1}, {0, 0}, 1.0},
    {{ 1, -1,  1}, {0, 0,  1}, {1, 0}, 1.0},
    {{-1,  1,  1}, {0, 0,  1}, {0, 1}, 1.0},
    {{ 1, -1,  1}, {0, 0,  1}, {1, 0}, 1.0},
    {{ 1,  1,  1}, {0, 0,  1}, {1, 1}, 1.0},
    {{-1,  1,  1}, {0, 0,  1}, {0, 1}, 1.0},

    // Back
    {{-1, -1, -1}, {0, 0, -1}, {0, 0}, 1.0},
    {{-1,  1, -1}, {0, 0, -1}, {0, 1}, 1.0},
    {{ 1, -1, -1}, {0, 0, -1}, {1, 0}, 1.0},
    {{ 1, -1, -1}, {0, 0, -1}, {1, 0}, 1.0},    
    {{-1,  1, -1}, {0, 0, -1}, {0, 1}, 1.0},
    {{ 1,  1, -1}, {0, 0, -1}, {1, 1}, 1.0},

    // Left
    {{-1, -1, -1}, {-1, 0, 0}, {0, 0}, 1.0},
    {{-1, -1,  1}, {-1, 0, 0}, {0, 1}, 1.0},
    {{-1,  1, -1}, {-1, 0, 0}, {1, 0}, 1.0},
    {{-1,  1, -1}, {-1, 0, 0}, {1, 0}, 1.0},
    {{-1, -1,  1}, {-1, 0, 0}, {0, 1}, 1.0},
    {{-1,  1,  1}, {-1, 0, 0}, {1, 1}, 1.0},

    // Right
    {{ 1, -1, -1}, { 1, 0, 0}, {0, 0}, 1.0},
    {{ 1,  1, -1}, { 1, 0, 0}, {1, 0}, 1.0},
    {{ 1, -1,  1}, { 1, 0, 0}, {0, 1}, 1.0},
    {{ 1,  1, -1}, { 1, 0, 0}, {1, 0}, 1.0},
    {{ 1,  1,  1}, { 1, 0, 0}, {1, 1}, 1.0},
    {{ 1, -1,  1}, { 1, 0, 0}, {0, 1}, 1.0},

    // Top
    {{-1, -1, -1}, { 0, -1, 0}, {0, 0}, 1.0},
    {{ 1, -1, -1}, { 0, -1, 0}, {1, 0}, 1.0},
    {{-1, -1,  1}, { 0, -1, 0}, {0, 1}, 1.0},
    {{ 1, -1, -1}, { 0, -1, 0}, {1, 0}, 1.0},
    {{ 1, -1,  1}, { 0, -1, 0}, {1, 1}, 1.0},
    {{-1, -1,  1}, { 0, -1, 0}, {0, 1}, 1.0},

    // Bottom
    {{-1,  1, -1}, { 0, 1, 0}, {0, 0}, 0.0},
    {{-1,  1,  1}, { 0, 1, 0}, {0, 1}, 0.0},
    {{ 1,  1, -1}, { 0, 1, 0}, {1, 0}, 0.0},
    {{ 1,  1, -1}, { 0, 1, 0}, {1, 0}, 0.0},
    {{-1,  1,  1}, { 0, 1, 0}, {0, 1}, 0.0},
    {{ 1,  1,  1}, { 0, 1, 0}, {1, 1}, 0.0},
};

float planeVertices[] = {
    -1000, 0, -1000,
    -1000, 0,  1000,
     1000, 0, -1000,
     1000, 0, -1000,
    -1000, 0,  1000,
     1000, 0,  1000,
};

void cubeAt(vertexInfo* buffer, glm::vec3 at, glm::vec3 size, float glow) {
    for(int i = 0; i < 36; i++) {
        vertexInfo cubeVert = cubeVertices[i];
        cubeVert.pos = cubeVert.pos * size + at;
        cubeVert.glow = glow;
        buffer[i] = cubeVert;
    }
}

void effectBlobsInitialize() {
    // Basic OpenGL state
    glClearColor(0.08f, 0.04f, 0.04f, 0.0f);
    
    // Cube drawing shader
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, "shaders/glowy_blobs.vert.glsl");
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "shaders/glowy_blobs_lighting.frag.glsl");
    shaderProgram = makeShaderProgram(fragmentShader, vertexShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Floor drawing shader
    vertexShader = loadShader(GL_VERTEX_SHADER, "shaders/basic.vert.glsl");
    fragmentShader = loadShader(GL_FRAGMENT_SHADER, "shaders/normalmap.frag.glsl");
    floorShaderProgram = makeShaderProgram(fragmentShader, vertexShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // House drawing shader
    vertexShader = loadShader(GL_VERTEX_SHADER, "shaders/basic.vert.glsl");
    fragmentShader = loadShader(GL_FRAGMENT_SHADER, "shaders/house_lighting.frag.glsl");
    houseShaderProgram = makeShaderProgram(fragmentShader, vertexShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Postprocessing
    blurShaderProgram = loadSaqShaderProgram("shaders/blur.frag.glsl");
    composeShaderProgram = loadSaqShaderProgram("shaders/glow_compose.frag.glsl");

    // Uniforms
    projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    modelviewLoc = glGetUniformLocation(shaderProgram, "modelview");
    normalviewLoc = glGetUniformLocation(shaderProgram, "normalview");

    // Attributes
    vertexInLoc = glGetAttribLocation(shaderProgram, "vertexIn");
    normalInLoc = glGetAttribLocation(shaderProgram, "normalIn");
    texcoordsInLoc = glGetAttribLocation(shaderProgram, "texcoordsIn");
    blobglowInLoc = glGetAttribLocation(shaderProgram, "blobglowIn");
    
    // Geometry
    blobVertices = (vertexInfo*)malloc(sizeof(vertexInfo) * 36 * BLOB_EXTENT * BLOB_EXTENT * BLOB_EXTENT);
    cubesBO = makeBO(GL_ARRAY_BUFFER, blobVertices, sizeof(vertexInfo) * 36 * BLOB_EXTENT * BLOB_EXTENT * BLOB_EXTENT, GL_DYNAMIC_DRAW);
    floorBO = makeBO(GL_ARRAY_BUFFER, planeVertices, sizeof(float) * 6 * 3, GL_STATIC_DRAW);

    std::vector<glm::vec3> houseVerts;
    std::vector<glm::vec2> houseUVs;
    std::vector<glm::vec3> houseNormals;
    loadOBJ("models/house2.obj", houseVerts, houseUVs, houseNormals);
    houseVertCount = houseVerts.size();

    vertexInfo* houseVertices = (vertexInfo*)malloc(sizeof(vertexInfo) * houseVertCount);
    for (int i = 0; i < houseVertCount; i++) {
        houseVertices[i].pos = houseVerts[i];
        houseVertices[i].texcoord = houseUVs[i];
        houseVertices[i].normal = houseNormals[i];
        houseVertices[i].glow = 1.0f;
    }
    houseBO = makeBO(GL_ARRAY_BUFFER, houseVertices, (GLsizei)(sizeof(vertexInfo) * houseVertCount), GL_STATIC_DRAW);

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
    brickTexture = loadTexture("texture/bricks_colour.tga");
    brickNormalTexture = loadTexture("texture/bricks_normals.tga");
    houseTexture = loadTexture("texture/house2.tga");

    postprocTextureInitial = makeTextureBuffer(screenWidth, screenHeight, GL_RGBA, GL_RGBA);
    postprocFBOInitial = makeFBO(postprocTextureInitial);

    postprocTextureA = makeTextureBuffer(screenWidth, screenHeight, GL_RGBA, GL_RGBA);
    postprocFBOA = makeFBO(postprocTextureA);

    postprocTextureB = makeTextureBuffer(screenWidth, screenHeight, GL_RGBA, GL_RGBA);
    postprocFBOB = makeFBO(postprocTextureB);
}

// Trefoil knot pathing
glm::vec3 blobPos(float t, int i) {
    if(i == 0) {
        return glm::vec3(sin(t) + 2.0f * sin(2.0f * t), cos(t) + 2.0f * cos(2.0f * t), -sin(3.0f * t)) * 2.85;
    }
    else if (i == 1) {
        return glm::vec3(-sin(3.0f * t), sin(t) + 2.0f * sin(2.0f * t), cos(t) + 2.0f * cos(2.0f * t)) * 2.85;
    }
    return glm::vec3(cos(t) + 2.0f * cos(2.0f * t), -sin(3.0f * t), sin(t) + 2.0f * sin(2.0f * t)) * 2.85;
}

// Draw a house
void drawHouse(glm::mat4 projection, glm::mat4 modelview, glm::mat4 cameraTransform) {
    // Calculate matrix for mirroring along the xz plane
    glm::mat4 modelviewMirrored = modelview;
    modelviewMirrored[1][1] = -modelviewMirrored[1][1];
    modelviewMirrored[3][1] = -modelviewMirrored[3][1];

    // Apply camera transform
    modelview = cameraTransform * modelview;
    modelviewMirrored = cameraTransform * modelviewMirrored;

    // Calculate normalview matrices
    glm::mat4 normalview = glm::transpose(glm::inverse(modelview));
    glm::mat4 normalviewMirrored = glm::transpose(glm::inverse(modelviewMirrored));

    glCullFace(GL_BACK);
    glUseProgram(houseShaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(houseShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(houseShaderProgram, "modelview"), 1, GL_FALSE, glm::value_ptr(modelview));
    glUniformMatrix4fv(glGetUniformLocation(houseShaderProgram, "normalview"), 1, GL_FALSE, glm::value_ptr(normalview));
    glUniformMatrix4fv(glGetUniformLocation(houseShaderProgram, "cameraTransform"), 1, GL_FALSE, glm::value_ptr(cameraTransform));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, houseTexture);
    glUniform1i(glGetUniformLocation(houseShaderProgram, "textureCol"), 0);

    glBindBuffer(GL_ARRAY_BUFFER, houseBO);

    GLuint houseVertexInLoc = glGetAttribLocation(houseShaderProgram, "vertexIn");
    GLuint houseNormalInLoc = glGetAttribLocation(houseShaderProgram, "normalIn");
    GLuint houseTexcoordInLoc = glGetAttribLocation(houseShaderProgram, "texcoordsIn");

    glEnableVertexAttribArray(houseVertexInLoc);
    glVertexAttribPointer(houseVertexInLoc, 3, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 0));

    glEnableVertexAttribArray(houseNormalInLoc);
    glVertexAttribPointer(houseNormalInLoc, 3, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 3));

    glEnableVertexAttribArray(houseTexcoordInLoc);
    glVertexAttribPointer(houseTexcoordInLoc, 2, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 6));

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)houseVertCount);

    // Draw mirrored
    glUniformMatrix4fv(glGetUniformLocation(houseShaderProgram, "modelview"), 1, GL_FALSE, glm::value_ptr(modelviewMirrored));
    glUniformMatrix4fv(glGetUniformLocation(houseShaderProgram, "normalview"), 1, GL_FALSE, glm::value_ptr(normalviewMirrored));

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)houseVertCount);

    glDisableVertexAttribArray(houseVertexInLoc);
    glDisableVertexAttribArray(houseNormalInLoc);
    glDisableVertexAttribArray(houseTexcoordInLoc);
}

void drawMetaballs(glm::mat4 projection, glm::mat4 modelview, glm::mat4 normalview, glm::mat4 modelviewMirrored, glm::mat4 normalviewMirrored, float bassRow) {
    // Bind cube shader and set up uniforms
    glUseProgram(shaderProgram);
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

                int blobState = (int)floor(((float)((int)bassRow % 16)) / 4.0);
                float blobGlow = 0.0f;
                if(rand() % 4 == blobState) {
                    blobGlow = bassRow / 4.0f;
                    blobGlow = 1.0f - (blobGlow - floor(blobGlow));
                    blobGlow = fmax(0.0f, blobGlow);
                }

                cubeAt(
                    &blobVertices[(x + y * BLOB_EXTENT + z * BLOB_EXTENT * BLOB_EXTENT) * 36], 
                    cubeCenter, 
                    glm::vec3((randFloat() * 0.3f + 0.5f) * blobValue),
                    blobGlow
                    );
            }
        }
    }

    // Set up texture
    glActiveTexture(GL_TEXTURE0);  
    glBindTexture(GL_TEXTURE_2D, glowyTexture);

    // Bind buffers and draw main thing
    glBindBuffer(GL_ARRAY_BUFFER, cubesBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexInfo) * 36 * BLOB_EXTENT * BLOB_EXTENT * BLOB_EXTENT, blobVertices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(vertexInLoc);
    glVertexAttribPointer(vertexInLoc, 3, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 0));

    glEnableVertexAttribArray(normalInLoc);
    glVertexAttribPointer(normalInLoc, 3, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 3));

    glEnableVertexAttribArray(texcoordsInLoc);
    glVertexAttribPointer(texcoordsInLoc, 2, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 6));

    glEnableVertexAttribArray(blobglowInLoc);
    glVertexAttribPointer(blobglowInLoc, 1, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 8));

    glDrawArrays(GL_TRIANGLES, 0, 36 * BLOB_EXTENT * BLOB_EXTENT * BLOB_EXTENT);

    // Draw the same again but mirrored
    glCullFace(GL_FRONT);
    glUniformMatrix4fv(modelviewLoc, 1, GL_FALSE, glm::value_ptr(modelviewMirrored));
    glUniformMatrix4fv(normalviewLoc, 1, GL_FALSE, glm::value_ptr(normalviewMirrored));
    glDrawArrays(GL_TRIANGLES, 0, 36 * BLOB_EXTENT * BLOB_EXTENT * BLOB_EXTENT);

    glDisableVertexAttribArray(vertexInLoc);
    glDisableVertexAttribArray(normalInLoc);
    glDisableVertexAttribArray(texcoordsInLoc);
    glDisableVertexAttribArray(blobglowInLoc);
}

// Draw a floor and blend
void drawFloor(glm::mat4 projection, glm::mat4 cameraTransform, glm::mat4 normalviewCamera) {
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);

    glCullFace(GL_BACK);
    glUseProgram(floorShaderProgram);

    glUniformMatrix4fv(glGetUniformLocation(floorShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(floorShaderProgram, "modelview"), 1, GL_FALSE, glm::value_ptr(cameraTransform));
    glUniformMatrix4fv(glGetUniformLocation(floorShaderProgram, "normalview"), 1, GL_FALSE, glm::value_ptr(normalviewCamera));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, brickTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, brickNormalTexture);

    glUniform1i(glGetUniformLocation(floorShaderProgram, "textureCol"), 0);
    glUniform1i(glGetUniformLocation(floorShaderProgram, "textureNorm"), 1);

    glBindBuffer(GL_ARRAY_BUFFER, floorBO);

    GLuint floorVertexInLoc = glGetAttribLocation(floorShaderProgram, "vertexIn");
    glEnableVertexAttribArray(floorVertexInLoc);
    glVertexAttribPointer(floorVertexInLoc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)(sizeof(float) * 0));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisable(GL_BLEND);
    glDisableVertexAttribArray(floorVertexInLoc);
}

// Actual effect
void effectBlobsRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, postprocFBOInitial);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    float bassRow = (float)bassGetRow(stream);
    
    // Standard perspective projection
    glm::mat4 projection = glm::perspective(90.0f, (float)screenWidth / (float)screenHeight, 0.1f, 200.0f);

    // Modelview shifts scene up and into the frame
    glm::mat4 modelview = glm::translate(glm::vec3(0.0f, 11.0f, 0.0f));

    // Calculate matrix for mirroring along the xz plane
    glm::mat4 modelviewMirrored = modelview;
    modelviewMirrored[1][1] = -modelviewMirrored[1][1];
    modelviewMirrored[3][1] = -modelviewMirrored[3][1];

    // Camera transform
    glm::mat4 cameraTransform = glm::translate(glm::vec3(0.0f, 0.0f, -25.0f)) * (glm::mat4)glm::quat(glm::vec3(sync_get_val(camRotX, bassRow), sync_get_val(camRotY, bassRow), sync_get_val(camRotZ, bassRow)));
    modelview = cameraTransform * modelview;
    modelviewMirrored = cameraTransform * modelviewMirrored;

    // Normalview -> Inverse transpose
    glm::mat4 normalview = glm::transpose(glm::inverse(modelview));
    glm::mat4 normalviewMirrored = glm::transpose(glm::inverse(modelviewMirrored));
    glm::mat4 normalviewCamera = glm::transpose(glm::inverse(cameraTransform));

    // Draw balls
    drawMetaballs(projection, modelview, normalview, modelviewMirrored, normalviewMirrored, bassRow);

    // Draw some houses
    int housesPerRing = 9;
    int rings = 40;
    for(int ring = 0; ring < rings; ring++) {
        float ringOffset = randFloat() * 2.0f * 3.141592f;
        for(int house = 0; house < housesPerRing; house++) {
            float angle = ((float)house / (float)housesPerRing) + (randFloat() * (0.8f / (float)housesPerRing)) + ringOffset;
            angle = angle * 2.0f * 3.141592f;
            float radius = ring * 6.5f + 30.0f;

            glm::mat4 houseModelview = glm::translate(glm::vec3(sin(angle) * radius, 0.0f, cos(angle) * radius));
            drawHouse(projection, houseModelview, cameraTransform);
        }
    }

    // Blend floor
    drawFloor(projection, cameraTransform,  normalviewCamera);

    // Do postproc
    glm::vec2 resolution = glm::vec2(screenWidth, screenHeight);
    glDisable(GL_DEPTH_TEST);
    for(int i = 0; i < 10; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, postprocFBOB);

        glActiveTexture(GL_TEXTURE0);
        if (i == 0) {
            glBindTexture(GL_TEXTURE_2D, postprocTextureInitial);
        }
        else {
            glBindTexture(GL_TEXTURE_2D, postprocTextureA);
        }

        glUseProgram(blurShaderProgram);
        glUniform2f(glGetUniformLocation(blurShaderProgram, "resolution"), (float)screenWidth, (float)screenHeight);
        glUniform2f(glGetUniformLocation(blurShaderProgram, "direction"), 0.0, 1.0);
        glUniform1i(glGetUniformLocation(blurShaderProgram, "tex"), 0);

        renderSAQ(blurShaderProgram);

        glBindFramebuffer(GL_FRAMEBUFFER, postprocFBOA);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, postprocTextureB);

        glUseProgram(blurShaderProgram);
        glUniform1i(glGetUniformLocation(blurShaderProgram, "tex"), 0);
        glUniform2f(glGetUniformLocation(blurShaderProgram, "resolution"), (float)screenWidth, (float)screenHeight);
        glUniform2f(glGetUniformLocation(blurShaderProgram, "direction"), 1.0, 0.0);

        renderSAQ(blurShaderProgram);
    }

    // Draw postproc quad to actual framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, postprocTextureInitial);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, postprocTextureA);

    glUseProgram(composeShaderProgram);
    glUniform1i(glGetUniformLocation(composeShaderProgram, "baseTex"), 0);
    glUniform1i(glGetUniformLocation(composeShaderProgram, "glowTex"), 1);

    renderSAQ(composeShaderProgram);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void effectBlobsTerminate() {
    // TODO proper this up
    // Textures
    glDeleteTextures(1, &brickTexture);
    glDeleteTextures(1, &brickNormalTexture);
    glDeleteTextures(1, &glowyTexture);
   
    // VBOs
    glDeleteBuffers(1, &cubesBO);
    glDeleteBuffers(1, &floorBO);

    // Shaders
    glDeleteProgram(shaderProgram);
    glDeleteProgram(blurShaderProgram);
    glDeleteProgram(composeShaderProgram);

    // TODO FBOS

    // Other data
    free(blobVertices);
}
