#include "bass_rocket.h"
#pragma comment(lib, "Ws2_32.lib")

#ifdef _WIN32
#include <direct.h>  
#endif

#include <stdio.h>  
#include <stdlib.h>  
#include <cmath>

#define CAN_SWITCH_SYNCMODE 1
#ifdef CAN_SWITCH_SYNCMODE
static bool syncClient = true;
#else
static const bool syncClient = true;
#endif

// Configure your song
static const float bpm = 125.0; /* beats per minute */
static const int rpb = 4; /* rows per beat */
static const double row_rate = 0.5; // (double(bpm) / 60) * rpb;

// A bunch of functions straight from the Rocket example code
void bassPause(void *d, int flag) {
    HSTREAM h = *((HSTREAM *)d);
    if (flag) {
        BASS_ChannelPause(h);
    } 
    else {
        BASS_ChannelPlay(h, false);
    }
}

void bassSetRow(void *d, int row) {
    HSTREAM h = *((HSTREAM *)d);
    int row_2 = row / row_rate;
    int low = row_2 / 64;
    int hi = (row_2 % 64) * 256.0;

    //QWORD pos = BASS_ChannelSeconds2Bytes(h, row / row_rate);
    BASS_ChannelSetPosition(h, low | (hi << 16), BASS_POS_MUSIC_ORDER);
}

int bassIsPlaying(void *d) {
    HSTREAM h = *((HSTREAM *)d);
    return BASS_ChannelIsActive(h) == BASS_ACTIVE_PLAYING;
}

double bassGetRow(HSTREAM h) {
    QWORD pos = BASS_ChannelGetPosition(h, BASS_POS_MUSIC_ORDER);
    //double time = BASS_ChannelBytes2Seconds(h, pos);
    //return time * row_rate;
    return (LOWORD(pos) * 64 + HIWORD(pos) / 256.0) * row_rate;
}

struct sync_cb bass_cb = {
    bassPause,
    bassSetRow,
    bassIsPlaying
};

// Start rocket
sync_device* syncStartup() {
    sync_device *rocket = sync_create_device("sync");

    if(syncClient) {
        if (sync_tcp_connect(rocket, "localhost", SYNC_DEFAULT_PORT)) {
            std::cerr << "Started in client mode, but could not connect to editor!" << std::endl;
#ifdef CAN_SWITCH_SYNCMODE
            std::cerr << "Switching to non-client mode." << std::endl;
            syncClient = false;
#else
            exit(-1);
#endif
        }
    }

    return rocket;
}

// Perform row update
void syncUpdate(sync_device* rocket, HSTREAM stream) {
    if(sync_update(rocket, (int)floor(bassGetRow(stream)), &bass_cb, (void *)&stream)) {
        if(syncClient) {
            if(sync_tcp_connect(rocket, "localhost", SYNC_DEFAULT_PORT)) {
                std::cerr << "Editor died" << std::endl;
                exit(-1);
            }
        }
    }
}
