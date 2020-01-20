#include "main.h"
#include "objloader.h"

#define PARTICLE_COUNT 32000
#define PARTICLE_SIZE 0.02f
#define PARTICLE_SPREAD 4.0f

#include "text2d.hpp"

static GLuint backgroundShaderProgram;
static GLuint trithingShaderProgram;
static GLuint particleShaderProgram;
static GLuint composeShaderProgram;

static GLuint renderTexture;
static GLuint depthTexture;
static GLuint renderFBO;

typedef struct vertexInfo {
    glm::vec3 pos;
    glm::vec3 normal;
} vertexInfo;

typedef struct particleInfo {
    glm::vec3 pos;
    glm::vec3 speed;
    float start;
} particleInfo;

typedef struct objectInfo {
    vertexInfo* vertices;
    glm::vec3 centroid;
    GLuint vertexBO;
    GLsizei vertCount;
    glm::vec3 color;
    glm::mat4 transform;
    const sync_track* scaleTrack;
    const sync_track* displaceTrack;
} objectInfo;

objectInfo objectParts[3];
vertexInfo* tempVerts;

vertexInfo* particleVerts;
particleInfo* particles;
GLuint particleBO;

// Geometry
objectInfo loadObject(const char* path, GLenum drawType = GL_STATIC_DRAW) {
    objectInfo newObject;

    std::vector<glm::vec3> loadVerts;
    std::vector<glm::vec2> loadUVs;
    std::vector<glm::vec3> loadNormals;
    
    loadOBJ(path, loadVerts, loadUVs, loadNormals);
    newObject.vertCount = (GLsizei)loadVerts.size();

    vertexInfo* loadVertices = (vertexInfo*)malloc(sizeof(vertexInfo) * newObject.vertCount);
    newObject.centroid = glm::vec3(0.0f);
    for (int i = 0; i < newObject.vertCount; i++) {
        loadVertices[i].pos = loadVerts[i];
        newObject.centroid += loadVerts[i];
        // loadVertices[i].texcoord = loadUVs[i];
        loadVertices[i].normal = loadNormals[i];
    }
    newObject.centroid /= newObject.vertCount;
    newObject.vertices = loadVertices;
    newObject.vertexBO = makeBO(GL_ARRAY_BUFFER, loadVertices, (GLsizei)(sizeof(vertexInfo) * newObject.vertCount), drawType);
    newObject.transform = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

    return newObject;
}

void effectTrithingInitialize() {
    // General OpenGL config
    glClearColor(0.1f, 0.0f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Shaders
    composeShaderProgram = loadSaqShaderProgram("shaders/trithing_compose.frag.glsl");

    const char* targets[] = {"outColor", "outDepth"};
    GLuint vertexShader = buildShader(GL_VERTEX_SHADER, getSaqVSSource());
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "shaders/trithing_background.frag.glsl");
    backgroundShaderProgram = makeShaderProgramMRT(vertexShader, fragmentShader, targets, 2);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    vertexShader = loadShader(GL_VERTEX_SHADER, "shaders/trithing_transform.vert.glsl");
    fragmentShader = loadShader(GL_FRAGMENT_SHADER, "shaders/trithing_shade.frag.glsl");
    trithingShaderProgram = makeShaderProgramMRT(vertexShader, fragmentShader, targets, 2);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    vertexShader = loadShader(GL_VERTEX_SHADER, "shaders/trithing_transform.vert.glsl");
    fragmentShader = loadShader(GL_FRAGMENT_SHADER, "shaders/trithing_shade_particles.frag.glsl");
    particleShaderProgram = makeShaderProgramMRT(vertexShader, fragmentShader, targets, 2);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // FBOs
    renderTexture = makeTextureBuffer(screenWidth, screenHeight, GL_RGBA32F);
    renderFBO = makeFBO(renderTexture);

    depthTexture = makeTextureBuffer(screenWidth, screenHeight, GL_RGBA32F);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, depthTexture, 0);

    // Geometry
    objectParts[0] = loadObject("models/trithing_hightri.obj", GL_DYNAMIC_DRAW);
    objectParts[0].scaleTrack = sync_get_track(rocket, "trithing:scale.outer");
    objectParts[0].displaceTrack = sync_get_track(rocket, "trithing:displace.inner");
    tempVerts = (vertexInfo*)malloc(sizeof(vertexInfo) * objectParts[0].vertCount);

    objectParts[1] = loadObject("models/trithing_innerA.obj");
    objectParts[1].scaleTrack = sync_get_track(rocket, "trithing:scale.middle");

    objectParts[2] = loadObject("models/trithing_innerB.obj");
    objectParts[2].scaleTrack = sync_get_track(rocket, "trithing:scale.inner");

    particleVerts = (vertexInfo*)malloc(sizeof(vertexInfo) * PARTICLE_COUNT);
    particles = (particleInfo*)malloc(sizeof(particleInfo) * PARTICLE_COUNT);

    for(int i = 0; i < PARTICLE_COUNT; i++) {
        particles[i].pos.x = randFloatUnit() * PARTICLE_SPREAD;
        particles[i].pos.y = randFloatUnit() * PARTICLE_SPREAD;
        particles[i].pos.z = randFloatUnit() * PARTICLE_SPREAD;

        particles[i].speed.x = randFloatUnit();
        particles[i].speed.y = randFloatUnit();
        particles[i].speed.z = randFloatUnit();
        particles[i].speed = glm::normalize(particles[i].speed) * 0.5f;

        particles[i].start = 0.0f;
    }

    particleBO = makeBO(GL_ARRAY_BUFFER, particleVerts, (GLsizei)(sizeof(vertexInfo) * PARTICLE_COUNT), GL_DYNAMIC_DRAW);
}

void effectTrithingRender() {
    // Grab sync info
    float bassRow = (float)bassGetRow(stream);
    static const sync_track* lightUp = sync_get_track(rocket, "trithing:lightUp");

    // Standard perspective projection
    glm::mat4 projection = glm::perspective(60.0f * (2.0f * 3.14159f / 180.0f), (float)screenWidth / (float)screenHeight, 0.1f, 200.0f);

    // Camera transform
    glm::mat4 cameraTransform = glm::translate(glm::vec3(0.0f, -0.5f, -6.0f)) * (glm::mat4)glm::quat(glm::vec3(0.8f, 0.0f, 0.0f));
    glm::mat4 cameraNormalview = glm::transpose(glm::inverse(cameraTransform));

    // Make sure textures are not bound
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Bind render FBO, enable MRT
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);

    GLenum targets[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, targets);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw background
    glDepthMask(GL_FALSE);
    glUseProgram(backgroundShaderProgram);

    glUniform1f(glGetUniformLocation(backgroundShaderProgram, "bassRow"), bassRow);
    glUniform1f(glGetUniformLocation(backgroundShaderProgram, "lightUp"), (GLfloat)sync_get_val(lightUp, bassRow));

    renderSAQ(backgroundShaderProgram);

    // Update trithing rotation / scale
    glm::mat4 trithingRotate = (glm::mat4)glm::quat(glm::vec3(0.0f, bassRow * 0.025f, 0.0f)); // TODO sync track me
    for(int i = 0; i < 3; i++) {
        float scaleVal = (float)sync_get_val(objectParts[i].scaleTrack, bassRow);
        glm::mat4 scale = glm::scale(glm::vec3(1.0f, (float)(scaleVal + 1.0f), 1.0f));
        glm::mat4 translate = glm::translate(glm::vec3(0.0f, -(float)(scaleVal + 1.0f) / 2.0f, 0.0f));
        objectParts[i].transform = trithingRotate * translate * scale;
    }

    // Update trithing displace
    float displace = (float)sync_get_val(objectParts[0].displaceTrack, bassRow);
    srand(666);
    for(int i = 0; i < objectParts[0].vertCount; i++) {
        glm::vec3 displaceDir = glm::normalize(objectParts[0].vertices[i].pos - objectParts[0].centroid);
        displaceDir.y *= 20.0f;
        tempVerts[i].pos = objectParts[0].vertices[i].pos + displaceDir * displace * randFloat();
        tempVerts[i].normal = objectParts[0].vertices[i].normal;
    }

    // Recalculate normals
    for(int i = 0; i < objectParts[0].vertCount; i += 3) {
        glm::vec3 s1 = objectParts[0].vertices[i + 1].pos - objectParts[0].vertices[i].pos;
        glm::vec3 s2 = objectParts[0].vertices[i + 2].pos - objectParts[0].vertices[i].pos;
        glm::vec3 n = glm::normalize(glm::cross(glm::normalize(s1), glm::normalize(s2)));
        objectParts[0].vertices[i].normal = n;
        objectParts[0].vertices[i + 1].normal = n;
        objectParts[0].vertices[i + 2].normal = n;
    }

    // Update resident vertices
    glBindBuffer(GL_ARRAY_BUFFER, objectParts[0].vertexBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexInfo) * objectParts[0].vertCount, tempVerts, GL_DYNAMIC_DRAW);

    // Draw trithing
    glDepthMask(GL_TRUE);
    glUseProgram(trithingShaderProgram);

    for(int i = 0; i < 3; i++) {
        glm::mat4 modelview = cameraTransform * objectParts[i].transform;
        glm::mat4 normalview = glm::transpose(glm::inverse(modelview));

        glUniformMatrix4fv(glGetUniformLocation(trithingShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(trithingShaderProgram, "modelview"), 1, GL_FALSE, glm::value_ptr(modelview));
        glUniformMatrix4fv(glGetUniformLocation(trithingShaderProgram, "normalview"), 1, GL_FALSE, glm::value_ptr(normalview));
        glUniformMatrix4fv(glGetUniformLocation(trithingShaderProgram, "cameraTransform"), 1, GL_FALSE, glm::value_ptr(cameraTransform));

        float shade = 0.5f + (float)i / 7.0f;
        glUniform4f(glGetUniformLocation(trithingShaderProgram, "color"), shade, shade, shade, 1.0f);
        glUniform4f(glGetUniformLocation(trithingShaderProgram, "colorGlow"), 0.0f, 0.0f, 0.0f, 0.0f);

        glBindBuffer(GL_ARRAY_BUFFER, objectParts[i].vertexBO);

        glEnableVertexAttribArray(glGetAttribLocation(trithingShaderProgram, "vertexIn"));
        glVertexAttribPointer(glGetAttribLocation(trithingShaderProgram, "vertexIn"), 3, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 0));

        glEnableVertexAttribArray(glGetAttribLocation(trithingShaderProgram, "normalIn"));
        glVertexAttribPointer(glGetAttribLocation(trithingShaderProgram, "normalIn"), 3, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 3));

        glDrawArrays(GL_TRIANGLES, 0, objectParts[i].vertCount);
    }

    // Update particles
    for(int i = 0; i < PARTICLE_COUNT; i++) {
        particleVerts[i].pos = particles[i].pos + particles[i].speed * (bassRow - particles[i].start - 0x140);
    }

    glBindBuffer(GL_ARRAY_BUFFER, particleBO);
    glBufferData(GL_ARRAY_BUFFER, (GLsizei)(sizeof(vertexInfo) * PARTICLE_COUNT), particleVerts, GL_DYNAMIC_DRAW);

    // Draw particles
    glEnable(GL_POINT_SPRITE);
    glPointSize(5.0f);

    glUseProgram(particleShaderProgram);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniformMatrix4fv(glGetUniformLocation(particleShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(particleShaderProgram, "modelview"), 1, GL_FALSE, glm::value_ptr(cameraTransform));
    glUniformMatrix4fv(glGetUniformLocation(particleShaderProgram, "normalview"), 1, GL_FALSE, glm::value_ptr(cameraNormalview));

    glUniform4f(glGetUniformLocation(particleShaderProgram, "colorGlow"), 100.0f * 0.2f, 100.0f * 0.8f, 100.0f * 1.0f, 0.0f);

    glBindBuffer(GL_ARRAY_BUFFER, particleBO);

    glEnableVertexAttribArray(glGetAttribLocation(particleShaderProgram, "vertexIn"));
    glVertexAttribPointer(glGetAttribLocation(particleShaderProgram, "vertexIn"), 3, GL_FLOAT, GL_FALSE, sizeof(vertexInfo), (void*)(sizeof(float) * 0));

    glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);
    glDisable(GL_BLEND);

    // Compose
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(composeShaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthTexture);

    glUseProgram(composeShaderProgram);
    glUniform1i(glGetUniformLocation(composeShaderProgram, "baseTex"), 0);
    glUniform1i(glGetUniformLocation(composeShaderProgram, "depthTex"), 1);

    renderSAQ(composeShaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void effectTrithingTerminate() {
    // Clean Geometry
    for(int i = 0; i < 3; i++) {
        free(objectParts[i].vertices);
        glDeleteBuffers(1, &objectParts[i].vertexBO);
    }
    free(tempVerts);

    glDeleteBuffers(1, &particleBO);
    free(particleVerts);

    // Clean programs
    glDeleteProgram(backgroundShaderProgram);
    glDeleteProgram(composeShaderProgram);

    // Clean FBOs
    glDeleteFramebuffers(1, &renderFBO);

    // Clean textures
    glDeleteTextures(1, &renderTexture);
}
