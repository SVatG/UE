/**
* OpenGL helper things for lazy people.
*/

#include "glhelpers.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
// "THIS FUNCTION IS UNSAFE"
#pragma warning(disable: 4996)
#endif

// Vertices for a single screen aligned quad
static GLuint saqBO = 0;

// Random float 0 -> 1
float randFloat() {
    return(static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
}

// Random float -1 -> 1
float randFloatUnit() {
    return(randFloat() * 2.0f - 1.0f);
}

// Simple helper to make a single buffer object.
GLuint makeBO(GLenum type, void* data, GLsizei size, int accessFlags) {
    GLuint bo;
    glGenBuffers(1, &bo);
    glBindBuffer(type, bo);
    glBufferData(type, size, data, accessFlags);
    return(bo);
}

// Helper function to make a buffer object of some size
GLuint makeTextureBuffer(int w, int h, GLenum format, GLint internalFormat) {
    GLuint buffertex;

    glGenTextures(1, &buffertex);
    glBindTexture(GL_TEXTURE_2D, buffertex);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_FLOAT, NULL);
    
    return buffertex;
}

// Function that makes an FBO with a 24bit Z buffer and no stencil buffer,
// with a texture attached to colour output 0.
GLuint makeFBO(GLuint texture) {
    // Determine texture size
    int width = 0;
    int height = 0;
    
    glBindTexture(GL_TEXTURE_2D, texture);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Make FBO and renderbuffer
    GLuint fbo;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    return fbo;
}

// Build a fullscreen shader
// The fragment program gets a "texcoord" input from 0 to 1.
GLuint buildSaqShaderProgram(GLchar* saqFSSource) {
    GLchar* saqVSSource = (GLchar*)"#version 150\n\nin vec2 pos;\nout vec2 texcoord;\nvoid main(void){gl_Position=vec4(pos*2.0f-1.0f, 0.0f, 1.0f);texcoord=pos;}\n\0";

    GLuint saqVS = buildShader(GL_VERTEX_SHADER, saqVSSource);
    GLuint saqFS = buildShader(GL_FRAGMENT_SHADER, saqFSSource);
    GLuint shaderProgram = makeShaderProgram(saqVS, saqFS);
    glDeleteShader(saqVS);
    glDeleteShader(saqFS);

    return(shaderProgram);
}

// As above, but load from file
GLuint loadSaqShaderProgram(const char* file) {
    GLchar* shaderSrc = loadFile(file);
    GLuint program = buildSaqShaderProgram(shaderSrc);
    free(shaderSrc);
    return program;
}

// Render a screen aligned quad with the given texture
void renderSAQ(GLuint saqShader) {
    glUseProgram(saqShader);
    if(saqBO == 0) {
        float saqVertices[] = {0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0};
        saqBO = makeBO(GL_ARRAY_BUFFER, saqVertices, sizeof(float) * 12, GL_STATIC_DRAW);
    }  
    
    GLuint saqVertLoc = glGetAttribLocation(saqShader, "pos");
    glBindBuffer(GL_ARRAY_BUFFER, saqBO);
    glEnableVertexAttribArray(saqVertLoc);
    glVertexAttribPointer(saqVertLoc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(saqVertLoc);
}

// Load text from a file.
char* loadFile(const char* name) {
    long size;
    char* buffer;
    FILE* fp = fopen(name, "rb");
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    buffer = (char*)malloc(size + 1);
    fseek(fp, 0, SEEK_SET);
    int res = (int)fread(buffer, 1, size, fp);
    if(res != size) {
        exit(-1);
    }
    fclose(fp);
    buffer[size] = '\0';
    return(buffer);
}

// Compile shader given directly
GLuint buildShader(GLenum type, GLchar* shaderSrc) {
    GLsizei length = (GLsizei)strlen(shaderSrc);
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&shaderSrc, &length);
    glCompileShader(shader);

    // Check for failure, display errors.
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    char log[65536];
    glGetShaderInfoLog(shader, 65535, 0, log);
    
    if(strlen(log) != 0) {
        fprintf(stderr, "Shader compile log:\n");
        fprintf(stderr, "%s", log);
    }
    
    if(status == 0) {
        fprintf(stderr, "(closing)\n");
        fgetc(stdin);
        exit(-1);
    }

    return shader;
}

// Load shader from a file.
GLuint loadShader(GLenum type, const char *file) {
    GLchar* shaderSrc = loadFile(file);
    GLuint shader = buildShader(type, shaderSrc);
    free(shaderSrc);
    return shader;
}

// Link a vertex shader and a fragment shader into a shader program.
GLuint makeShaderProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check for failure, display errors.
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == 0) {
        fprintf(stderr, "Shader link error.\n");
        char log[65536];
        glGetProgramInfoLog(program, 65535, &status, log);
        fprintf(stderr, "%s", log);
        fgetc(stdin);
        exit(-1);
    }

    return program;
}

static short le_short(unsigned char *bytes) {
    return bytes[0] | ((char)bytes[1] << 8);
}

// Read a tga file into a buffer for use as an OpenGL texture.
// Original code by Joe Groff, modified to handle alpha channels.
void *readTga(const char *filename, int *width, int *height, int *alpha) {

    // TGA header for loading things into.
    struct tga_header {
        char id_length;
        char color_map_type;
        char data_type_code;
        unsigned char color_map_origin[2];
        unsigned char color_map_length[2];
        char color_map_depth;
        unsigned char x_origin[2];
        unsigned char y_origin[2];
        unsigned char width[2];
        unsigned char height[2];
        char bits_per_pixel;
        char image_descriptor;
    } header;

    size_t color_map_size, pixels_size;
    FILE *f;
    size_t read;
    void *pixels;

    // Try to open the file.
    f = fopen(filename, "rb");
    if(!f) {
        fprintf(stderr, "Unable to open %s for reading\n", filename);
        return NULL;
    }

    // Check for valid header data.
    read = fread(&header, 1, sizeof(header), f);
    if(read != sizeof(header)) {
        fprintf(stderr, "%s has incomplete tga header\n", filename);
        fclose(f);
        return NULL;
    }
    if(header.data_type_code != 2) {
        fprintf(
            stderr,
            "%s is not an uncompressed RGB tga file\n",
            filename
            );
        fclose(f);
        return NULL;
    }
    if((header.bits_per_pixel != 32) && (header.bits_per_pixel != 24)) {
        fprintf(
            stderr,
            "%s is not 24/32-bit uncompressed RGB/A tga file.\n",
            filename
            );
        fclose(f);
        return NULL;
    }

    // Return to the outside if an alpha channel is present.
    if(header.bits_per_pixel == 32) {
        *alpha = 1;
    }
    else {
        *alpha = 0;
    }

    // Only handling non-palleted images.
    color_map_size =
        le_short(header.color_map_length) * (header.color_map_depth/8);
    if(color_map_size > 0) {
        fprintf(
            stderr,
            "%s is colormapped, cannot handle that.\n",
            filename
            );
        fclose(f);
        return NULL;
    }

    // Set return width/height values and calculate image size.
    *width = le_short(header.width);
    *height = le_short(header.height);
    pixels_size = *width * *height * (header.bits_per_pixel / 8);
    pixels = malloc(pixels_size);

    // Read image.
    read = fread(pixels, 1, pixels_size, f);
    if(read != pixels_size) {
        fprintf(stderr, "%s has incomplete image\n", filename);
        fclose(f);
        free(pixels);
        return NULL;
    }

    return pixels;
}

// Load a texture from a TGA file.
GLuint loadTexture(const char *filename) {	

    // Load tga data into buffer.
    int width, height, alpha;
    unsigned char* pixels;

    pixels = (unsigned char*)readTga(filename, &width, &height, &alpha);
    if(pixels == 0) {
        fprintf(stderr, "Image loading failed: %s\n", filename);
        return 0;
    }

    // Generate texture, bind as active texture.
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Load pixels from buffer into texture.
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        alpha == 1 ? GL_RGBA8 : GL_RGB8,
        width,
        height,
        0,
        alpha == 1 ? GL_BGRA : GL_BGR,
        GL_UNSIGNED_BYTE,
        pixels
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    // Release buffer.
    free(pixels);

    return texture;
}


// Generate a float texture from given data
GLuint genFloatTexture(float *data, int width, int height) {
    // Generate texture, bind as active texture.
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    // Load pixels from buffer into texture.
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_R32F,
        width,
        height,
        0,
        GL_RED,
        GL_FLOAT,
        data
        );

    return texture;
}

// Debug context log printer
void printDebugLog(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, const char* message) {
    char debSource[16], debType[20], debSev[8];
    if(source == GL_DEBUG_SOURCE_API)
        strcpy(debSource, "OpenGL");
    else if(source == GL_DEBUG_SOURCE_WINDOW_SYSTEM)
        strcpy(debSource, "Windows");
    else if(source == GL_DEBUG_SOURCE_SHADER_COMPILER)
        strcpy(debSource, "Shader Compiler");
    else if(source == GL_DEBUG_SOURCE_THIRD_PARTY)
        strcpy(debSource, "Third Party");
    else if(source == GL_DEBUG_SOURCE_APPLICATION)
        strcpy(debSource, "Application");
    else if(source == GL_DEBUG_SOURCE_OTHER)
        strcpy(debSource, "Other");

    if(type == GL_DEBUG_TYPE_ERROR)
        strcpy(debType, "Error");
    else if(type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
        strcpy(debType, "Deprecated behavior");
    else if(type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
        strcpy(debType, "Undefined behavior");
    else if(type == GL_DEBUG_TYPE_PORTABILITY)
        strcpy(debType, "Portability");
    else if(type == GL_DEBUG_TYPE_PERFORMANCE)
        strcpy(debType, "Performance");
    else if(type == GL_DEBUG_TYPE_OTHER)
        strcpy(debType, "Other");

    if(severity == GL_DEBUG_SEVERITY_HIGH)
        strcpy(debSev, "High");
    else if(severity == GL_DEBUG_SEVERITY_MEDIUM)
        strcpy(debSev, "Medium");
    else if(severity == GL_DEBUG_SEVERITY_LOW)
        strcpy(debSev, "Low");

    printf("GL: Source:%s\tType:%s\tID:%d\tSeverity:%s\tMessage:%s\n", debSource, debType, id, debSev, message);
}

// Debug context log callback
unsigned int glDebugLogLevel = GL_DEBUG_SEVERITY_LOW;
#ifdef WIN32
void __stdcall debugCallback(
#else
void debugCallback(
#endif
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    GLvoid* userParam
    ) {
    if((glDebugLogLevel == GL_DEBUG_SEVERITY_LOW) ||
        ((glDebugLogLevel == GL_DEBUG_SEVERITY_MEDIUM) && (severity != GL_DEBUG_SEVERITY_LOW)) ||
        (severity == GL_DEBUG_SEVERITY_HIGH)) {
        printDebugLog(source, type, id, severity, message);
    }
    if(severity == GL_DEBUG_SEVERITY_HIGH) {
        getc(stdin);
    }
}

// Debug context logger registration
void registerGlDebugLogger(unsigned int logLevel) {
    glDebugLogLevel = logLevel;
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLARBPROC) glfwGetProcAddress("glDebugMessageControlARB");
    glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKARBPROC) glfwGetProcAddress("glDebugMessageCallbackARB");
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    glDebugMessageCallback((GLDEBUGPROC)&debugCallback, NULL);
}
