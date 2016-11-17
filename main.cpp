#include "main.h"

// Global state
static GLFWwindow* window = NULL;

// Basic error handler
static void glfwErrorHandler(int error, const char* description) {
    std::cerr << "Error " << error << ": " << description << std::endl;
    exit(-1);
}

// Basic input handler ("escape closes window")
static void glfwInputHandler(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

// Startup
static void initializeApplication() {
    // Start up GLFW and set up error handling
    if (!glfwInit()) {
        printf("Error in init\n");
        exit(-1);
    }
    glfwSetErrorCallback(glfwErrorHandler);
    
    // Open a window and set up OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window = glfwCreateWindow(screenWidth, screenHeight, "extremely basic window", NULL, NULL);
    
    // Set up input handling
    glfwSetKeyCallback(window, glfwInputHandler);
    
    // Set up OpenGL
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);    
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);
}



// Teardown
static void terminateApplication() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Main
int main() {
    initializeApplication();
    
    // Demo main loop
    while (!glfwWindowShouldClose(window)) {
        // TODO: anything that actually renders
        // TODO: music playing
        // TODO: postprod / fbos
        // TODO: rocket init / integration
        
        // Draw to screen
        glfwSwapBuffers(window);
        
        // Allow GLFW to do event handling
        glfwPollEvents();
    }
    
    terminateApplication();
}
