#pragma once

// Includes
#include <glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

char* loadFile(char* name);
float randFloat();
float randFloatUnit();

GLuint genFloatTexture(float *data, int width, int height);

GLuint makeBO(GLenum type, void* data, GLsizei size, int accessFlags);
GLuint makeTextureBuffer(int w, int h, GLenum format, GLint internalFormat);

GLuint loadShader(GLenum type, char *file);
GLuint makeShaderProgram(GLuint vertexShader, GLuint fragmentShader);

GLuint loadTexture(const char *filename);

void registerGlDebugLogger(unsigned int logLevel);
