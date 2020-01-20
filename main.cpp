#include "main.h"
#include "effects.h"
#include "text2d.hpp"

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
    window = glfwCreateWindow(screenWidth, screenHeight, "extremely basic window", glfwGetPrimaryMonitor(), NULL);

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
int main(int argc, char** argv) {
    initializeApplication();
    

    // Music!
    //stream = BASS_StreamCreateFile(false, "grey_matter.it", 0, 0, BASS_STREAM_PRESCAN);
    //stream = BASS_StreamCreateFile(false, "sbit5.ogg", 0, 0, BASS_STREAM_PRESCAN);
    stream = BASS_MusicLoad(false, "grey_matter.it", 0, 0, BASS_MUSIC_PRESCAN | BASS_SAMPLE_LOOP, 0);
    BASS_ChannelSetAttribute(stream, BASS_ATTRIB_MUSIC_PSCALER, 256);
    BASS_Start();
    BASS_ChannelPlay(stream, false);

    int curEffect = 0;

    effectBlobsInitialize();

    const sync_track* switch_track = sync_get_track(rocket, "global:switch");
    initText2D("texture/font.tga");

    // Demo main loop
    while (!glfwWindowShouldClose(window)) {
        // TODO: postprod / fbos
        syncUpdate(rocket, stream);
        float bassRow = (float)bassGetRow(stream);

        // Switch
        int nextEffect = sync_get_val(switch_track, bassRow);
        if (nextEffect != curEffect) {
            if (curEffect == 1) {
                curEffect = 0;
                effectTrithingTerminate();
                effectBlobsInitialize();
            }
            else {
                curEffect = 1;
                effectBlobsTerminate();
                effectTrithingInitialize();
            }
        }

        // Test effect
        if (curEffect == 0) {
            effectBlobsRender();
        }
        else {
            effectTrithingRender();
        }

        // Text!
        glClear(GL_DEPTH_BUFFER_BIT);
        printText2D("this excellent SVatG party production has been brought to you by halcy and Saga Musix. How many SVatG engineers does it take to create a bitmap font? Two.               Greetings to: k2 ~ TiTAN ~ SunSpire ~ cncd ~ Nuance ~ logicoma ~ mercury ~ spacepigs ~ jvb ~ Poo-Brain ~ RNO ~ dotUser ~ Suricrasia Online ~ Alcatraz ~ Wursthupe ~ Kaltgetraenkekabel                    We hope you have a great party here at Nordlicht 2019 in sunny Bremen (ha ha)                this scroller will now repeat...", -bassRow * 35.0 + 3000, sin(bassRow) * 60 + 120, 30);

        // Draw to screen
        glfwSwapBuffers(window);
        
        // Allow GLFW to do event handling
        glfwPollEvents();
    }

    // Test effect
    if (curEffect == 0) {
        effectBlobsTerminate();
    }
    else {
        effectTrithingTerminate();
    }

    terminateApplication();
}
