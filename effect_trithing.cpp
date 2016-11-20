#include "main.h"
#include "objloader.h"

static GLuint backgroundShaderProgram;
static GLuint composeShaderProgram;

static GLuint renderTexture;
static GLuint renderFBO;

void effectTrithingInitialize() {
    // General OpenGL config
    glClearColor(0.1f, 0.0f, 0.1f, 1.0f);

    // Shaders
    backgroundShaderProgram = loadSaqShaderProgram("shaders/trithing_background.frag.glsl");
    composeShaderProgram = loadSaqShaderProgram("shaders/trithing_compose.frag.glsl");

    // FBOs
    renderTexture = makeTextureBuffer(screenWidth, screenHeight, GL_RGBA, GL_RGBA);
    renderFBO = makeFBO(renderTexture);
}

void effectTrithingRender() {
    float bassRow = bassGetRow(stream);

    // Draw background
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(backgroundShaderProgram);

    glUniform1f(glGetUniformLocation(backgroundShaderProgram, "bassRow"), bassRow);

    renderSAQ(backgroundShaderProgram);

    // Compose
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glUseProgram(composeShaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTexture);

    glUseProgram(composeShaderProgram);
    glUniform1i(glGetUniformLocation(composeShaderProgram, "baseTex"), 0);

    renderSAQ(composeShaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void effectTrithingTerminate() {
   
}
