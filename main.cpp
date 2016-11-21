#include "main.h"
#include "effects.h"

// Global state
static GLFWwindow* window = NULL;
sync_device* rocket;
HSTREAM stream;

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

    // Set up BASS
    BASS_Init(-1, 44100, 0, 0, 0);

    // Set up rocket
    rocket = syncStartup();
}

// Teardown
static void terminateApplication() {
    sync_destroy_device(rocket);
    BASS_StreamFree(stream);
    BASS_Free();
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Main
int main() {
    initializeApplication();
    
    // Music!
    stream = BASS_StreamCreateFile(false, "sbit5.ogg", 0, 0, BASS_STREAM_PRESCAN);
    BASS_Start();
    BASS_ChannelPlay(stream, false);

    effectBlobsInitialize();
//     effectTrithingInitialize();

    // Demo main loop
    while (!glfwWindowShouldClose(window)) {
        // TODO: postprod / fbos
        syncUpdate(rocket, stream);

        // Test effect
        effectBlobsRender();
//         effectTrithingRender();

        // Draw to screen
        glfwSwapBuffers(window);
        
        // Allow GLFW to do event handling
        glfwPollEvents();
    }
    
    effectBlobsTerminate();
//     effectTrithingTerminate();

    terminateApplication();
}
